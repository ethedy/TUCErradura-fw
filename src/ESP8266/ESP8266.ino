#include <sha256.h>


/*
  ESP8266.INO

  Autor: 
  Fecha: NOV-DIC/2024
  Version: 1

  Controlador de cerradura electronica para TUSE

  CAMBIOS

  20/12/2024 ET - Cambio delay en loop por scheduled functions, una feature del "OS" del ESP8266

  12/01/2025 ET - Cambio vector<T> por forward_list<T> por mejor uso de memoria y no necesitamos algunas de las features de vector

  17/01/2025 ET - UserSession
                  - cambio LastLogin tipo long por std::chrono::milliseconds LastAccessAbsolute 
                  - cambio String Token por uint64_t Token para guardarlo numerico y que sea mas eficiente la comparacion
                  - cambio String Rol por Roles Rol, usando un enum que es mas facil de manipular en el codigo
                - GenerarToken() retorna un uint64_t considerando que millis() es unsigned long (32 bits) y estoy sumandole un valor random
                  es necesario que el resultado sea de un orden superior

  20/01/2025 ET - Incorporamos funcion Authorize() con un argumento que nos permite establecer una policy de autorizacion para el endpoint
                  desde el que lo llamamos. Revisa el token, verifica que exista, en caso de no existir se retorna un no-autenticado, y si
                  existe y el usuario esta en el cache de sesiones se autoriza la solicitud, en otro caso se revoca para que se solicite un
                  nuevo login

  26/01/2025 ET - Authorize() retorna un std::variant porque necesitamos conocer por que fallo...
                  Agregamos salida JSON a login

  30/01/2025 ET - Finalmente procesamos la fecha-hora del header en formato [YYYY-MM-DD HH:mm:ss] estricto, o sea con esos separadores, ese tamaño
                  y ese formato. Lamentablemente no podemos ponernos exquisitos porque el codigo crece o si usamos regex nos arriesgamos a que
                  haya recursiones
                  Seteamos la hora del chip con settimeofday() correctamente y se mantiene estable

*/
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <Schedule.h>
#include <flash_hal.h>
#include <sha256.h>
#include <ArduinoJson.h>
#include <EEPROM.h>

#include <time.h>
#include <limits.h>
#include <forward_list>
#include <utility>
#include <chrono>
#include <variant>

#include "Servicios.h"
#include "Utiles.h"
#include "Entidades.h"

using namespace Servicios;
using namespace std::chrono_literals;
using namespace std::string_literals;

/*
  ------------------------------------------------
          Constantes y Definiciones
  ------------------------------------------------
*/

#define SHA256_SIZE 32  // El tamaño del hash SHA256 es 32 bytes

//unsigned long sessionLimit = 900000;  // 15 minutos de tiempo para session activa
//unsigned long lastLoginTime = 0;      // Para gestionar la expiración de la sesión

const uint8_t Door_Pin = 2;                 // Pin del LED
const uint8_t Watchdog_Pin = 2;
const uint8_t Feedback_Pin = 2;         //  led para feedback al usuario respecto a los intentos de conexion

const uint8_t Feedback_WiFiConnect_Delay = 2;
const uint8_t Feedback_AccessPoint_Delay = 14;

//  pasados a Config.h y renombrados
// const uint32_t Watchdog_Time = 10 * 1'000'000;          //  pasarlo a segundos
// const uint32_t ClearSession_Time = 25 * 1'000'000UL;    //  pasarlo a segundos

const char* Datetime_Header = "X-Tuse-Datetime";
const char* Token_Header = "X-Tuse-Token";

const uint64_t Token_Universal = 0x6DA1C9B10971F981;

/*
  ------------------------------------------------
                Tipos y Variables
  ------------------------------------------------
*/

//  extern uint32_t EEPROM_start;

ESP8266WebServer server(80);

PersistentConfig Configuration;

// struct User {
//   String User;
//   String PassHash;  // Usaremos el hash de la contraseña
//   String Rol;       // "admin" o "user"
// };

//  std::vector<UserSession> UsuariosLogueados;  // Vector para almacenar los usuarios logueados

//  cambio vector<T> por forward_list<T>, es mas simple (lista enlazada) no necesitamos que sea bidireccional ni de acceso aleatorio
//
std::forward_list<UserSession> Sesiones;

std::function<bool(UserSession)> Policy_Admin = [](UserSession us) {
  return us.Rol == Roles::Admin_Rol;
};

inline bool g_enable_feedback = false;

inline uint8_t g_feedback_count = 1;

UserSession GenericSesion { "t800@skynet.gov", "Sarah Connor", Roles::Unknown_Rol };

/*
  ------------------------------------------------
            Declaracion de end-points
  ------------------------------------------------
*/

void HandleInfo();

void HandleLogin();

void HandleOpenDoor();

void HandleAddUser();

//void HandleGetUser();

//void HandleUpdateUser();

void HandleDeleteUser();

void HandleFactoryReset();

void SendErrorResponse();

/*
  ------------------------------------------------
            Declaracion de funciones
  ------------------------------------------------
*/

uint64_t GenerarToken();

String GenerarHash(const String& user_pass);

void MostrarUsuariosLogueados();

void PurgarTokensExpirados();

bool ValidarSesion(const String& token, const UserSession& sesion);

bool IsTokenOutdated(const UserSession& sesion);

void TryProcessDatetimeHeader();

void TrySetupDateTime();

std::variant<AuthorizationErrors, UserSession>
Authorize(std::function<bool(UserSession)> policy = nullptr);

void ShowHardwareInfo(AuthorizationErrors error);

void FactoryReset();

/*
  ------------------------------------------------
            Definicion de end-points
  ------------------------------------------------
*/

/*
  Retorna informacion del chip y del firmware


*/
void HandleInfo()
{
  auto sesion = Authorize(Policy_Admin);

  TryProcessDatetimeHeader();

  if (std::holds_alternative<UserSession>(sesion))
  {
    //  ok, seguimos...
    //  UserSession us = std::get<UserSession>(sesion);
    InfoResult result {};

    result.MacAddress = WiFi.macAddress().c_str();

    server.send(200, "application/json", result.ToJson().c_str());
    return;
  }
  else
  {
    SendErrorResponse(std::get<AuthorizationErrors>(sesion));
  }
}

/* 
  Función para manejar la solicitud de login

  Es el unico metodo que no necesita autorizacion, es decir que si no encuentra el token se continua con el proceso
*/
void HandleLogin() 
{
  auto sesion = Authorize();

  TryProcessDatetimeHeader();

  if (std::holds_alternative<UserSession>(sesion)) 
  {
    LOG_VERBOSE("OMG usuario ya en cache que quiere loguearse nuevamente??");

    Entidades::LoginResult okResult { 200, "Usuario ya conectado" };

    server.send(200, "application/json", okResult.ToJson().c_str());
    return;
  }

  //  nuevo login o token expirado
  //  este es el unico endpoint en el que si no retornamos una sesion, se puede continuar ya que tenemos que completar el procedo de login

  if (server.hasArg("user") && server.hasArg("pass")) 
  {
    String User = server.arg("user");
    String Pass = server.arg("pass");
    
    ServiciosSeguridad seguridad;
    UserSession nuevaSesion = seguridad.LoginUser(User, Pass);

    if (nuevaSesion.StatusCode == 0) 
    {
      //  Nos aseguramos que el mismo usuario no se loguee dos veces!
      //
      auto find = std::find_if(Sesiones.begin(), Sesiones.end(), [&nuevaSesion](UserSession us) { return us.Email == nuevaSesion.Email; });

      if (find != Sesiones.end())
      {
        LOG_INFO("Credenciales duplicadas. Eliminando sesion previa del mismo user");

        //  eliminar la sesion previa duplicada, para ello usamos una triquiñuela con swap y pop_front ya que forward_list 
        //  no dispone de un metodo para eliminar el iterador actual
        //
        std::swap(*find, Sesiones.front());
        Sesiones.pop_front();
      }
      //  Generamos un nuevo token para este usuario y lo añadimos
      //
      unsigned long ms = millis();

      nuevaSesion.Token = GenerarToken();
      nuevaSesion.LastAccessAbsolute = std::chrono::milliseconds(ms) ;

      //  LOG_VERBOSE("ms: %lu - convertido: %llu", ms, nuevaSesion.LastAccessAbsolute.count());

      //  usuario.LastLogin = millis();  // Marcar el tiempo del último login
      //  para mi tenemos que setear el tiempo de timeout segun admin o user y luego decrementarlo en cada pasada

      // Si es válido, lo agregamos al vector de usuarios logueados
      //  UsuariosLogueados.push_back(usuario);
      
      Sesiones.push_front(nuevaSesion);

      Entidades::LoginResult okResult { 200, "Login exitoso" };

      okResult.Token = nuevaSesion.Token;
      server.send(200, "application/json", okResult.ToJson().c_str());
    } 
    else 
    {
      if (nuevaSesion.StatusCode == ValidationErrors::Invalid_Credentials || nuevaSesion.StatusCode == ValidationErrors::User_Not_Found)
      {
        Entidades::LoginResult badResult { 401, "Credenciales incorrectas. No autenticado" };

        server.send(401, "application/json", badResult.ToJson().c_str());
      }
      else
      {
        if (nuevaSesion.StatusCode == ValidationErrors::Bad_Formed_User_Password)
        {
          Entidades::LoginResult badResult { 400, "El mail y/o contraseña no respetan el formato especificado por la aplicacion" };

          server.send(400, "application/json", badResult.ToJson().c_str());
        }
      }
    }
  } 
  else 
  {
    Entidades::LoginResult badResult { 400, "Parámetros de usuario y contraseña faltantes" };

    server.send(400, "application/json", badResult.ToJson().c_str());
  }
}


// Lógica para abrir la puerta
void HandleOpenDoor() 
{
  UserSession usuario;

  if (server.hasArg("Token")) 
  {
    if (ValidarSesion(server.arg("Token"), usuario)) 
    {
      //  TODO sacar ValidarSesion
      //  TODO agregar servicio de validacion de horario

      // El usuario está logueado, proceder a abrir la puerta
      digitalWrite(Door_Pin, LOW);  // Abre la puerta
      server.send(200, "text/plain", "Puerta abierta exitosamente.");
      delay(5000);                 // Mantiene la puerta abierta durante 5 segundos????
      digitalWrite(Door_Pin, HIGH);  // Cierra la puerta
      return;
    } 
    else 
    {
      // El usuario no está logueado, mostramos un mensaje de error
      server.send(401, "text/plain", "Error(1000): Usuario no autenticado.");
    }
  } 
  else 
  {
    // Si no se recibe el parámetro 'user' o 'token', mostramos un mensaje de error
    server.send(401, "text/plain", "Error: Parámetros 'user' o 'token' faltantes.");
  }
}


/*
  Funcion para manejar el request de agregado de usuario

  /adduser (email, pwd, nombre, rol)

*/
void HandleAddUser() 
{
  auto sesion = Authorize(Policy_Admin);

  TryProcessDatetimeHeader();

  if (std::holds_alternative<UserSession>(sesion))
  {
    UserSession us = std::get<UserSession>(sesion);

  }
  else
  {
    SendErrorResponse(std::get<AuthorizationErrors>(sesion));
  }

/*
  if (server.hasArg("Token")) 
  {
    if (ValidarSesion(server.arg("Token"), usuario)) 
    {
      //  if (usuario.Rol == "admin") 
      if (usuario.Rol == Roles::Admin_Rol)
      {

        //llamar funcion de Thedy (addUser)

        // Si el usuario no existe, agregamos el nuevo usuario
        // User newUser;
        // newUser.User = newUser.User;
        // newUser.PassHash = generarHash(newPass); // Guardamos el hash de la contraseña
        // newUser.Rol = newRol;

        // if (SaveNewUser)
        // {
        //      server.send(200, "text/plain", "Usuario agregado exitosamente.");
        // }
        // else
        // {
        //     server.send(400, "text/plain", "Erro(1005): Usario no agregado")
        // }
      }
    }
    return;
  }
*/  
}


// Función para eliminar un usuario
void HandleDeleteUser() 
{
  if (server.hasArg("user")) 
  {
    String userToDelete = server.arg("user");

    //  TODO forward_list
    //
    // Buscar el usuario en el vector
    // for (size_t i = 0; i < UsuariosLogueados.size(); i++) 
    // {
    //   if (UsuariosLogueados[i].User == userToDelete) 
    //   {
    //     // Eliminar el usuario encontrado
    //     UsuariosLogueados.erase(UsuariosLogueados.begin() + i);
    //     server.send(200, "text/plain", "Usuario eliminado exitosamente.");
    //     return;
    //   }
    // }

    // Si el usuario no fue encontrado
    server.send(404, "text/plain", "Error: Usuario no encontrado.");
  } 
  else 
  {
    server.send(400, "text/plain", "Error: Parámetro 'user' faltante.");
  }
}

/*
  Usar con sumo cuidado!!!

  Esto elimina toda la configuracion que tiene que ver con
  - databases
  - EEPROM
  - wifi manager
*/
void HandleFactoryReset()
{
  auto sesion = Authorize(Policy_Admin);

  if (std::holds_alternative<UserSession>(sesion))
  {
    //  todo perfecto...
    //  no es un error pero es critico
    LOG_ERROR("Se va a restaurar el seteo a los valores de fabrica, se pierde TODO!!");

    FactoryReset();
  }
  else
    SendErrorResponse(std::get<AuthorizationErrors>(sesion));
}


/*
  Si bien no es un end-point la vamos a usar en la mayoria de los end-points para enviar respuestas que involucran errores

    Token_Not_Found,
    Token_Expired,
    Token_Invalid,
    No_Autorizado,
    No_Autenticado
*/
void SendErrorResponse(AuthorizationErrors error)
{
  ProblemDetailResult resultado;

  switch (error)
  {
    case AuthorizationErrors::No_Autenticado:
      resultado.Status = 401;
      resultado.Title = "Tenes que autenticarte para utilizar nuestros servicios. Por favor dirigirse a la opcion de login"s;
      break;

    case AuthorizationErrors::No_Autorizado:
      resultado.Status = 403;
      resultado.Title = "No estas autorizado a usar la opcion solicitada. Consulta un administrador"s;
      break;

    default:
      resultado.Status = 400;
      resultado.Title = "Falta el token de autenticacion o es invalido"s;
      break;
  }

  server.send(resultado.Status, "application/json", resultado.ToJson().c_str());
}

/*
  ------------------------------------------------
            Definicion de Funciones
  ------------------------------------------------
*/

/*
  Función para generar un token único
  Genera un token para retornar al usuario y para conservar en el cache de sesiones para proximas solicitudes

  Cambiamos de String a uint64_t para mejorar los tiempos de comparacion
*/
uint64_t GenerarToken() 
{
  //  String token = String(millis(), HEX) + String(random(0, 255), HEX);  // Usamos el tiempo y un valor aleatorio como token
  //  return token;
  unsigned long ms = millis();
  long rnd = random(0, 65535);
  uint64_t result = (static_cast<uint64_t>(ms) << 16) + rnd;

  //  64 bits son 16 caracteres hexadecimales...
  //
  LOG_INFO("milis: %lu, rand: %li, token: 0x%016llx", ms, rnd, result);

  return result;
}


/*
  Función para generar el hash SHA256 de la contraseña utilizando la libreria de AWS

String GenerarHash(const String& user_pass) 
{
  SHA256 sha256;   // Crear un objeto de la clase SHA256
  sha256.reset();  // Reiniciar el objeto SHA256 (equivalente a init())

  // Usar `add()` para agregar los datos (en este caso la contraseña)
  sha256.add((const void*)user_pass.c_str(), user_pass.length());

  // Obtener el hash resultante como un string hexadecimal
  char* hash = sha256.getHash();  // Obtiene el hash como un string hexadecimal

  //  256 bits representan 32 bytes o sea que entraria en un char[32]
  //
  return hash;

  // Convertir el hash hexadecimal a un String y retornarlo

  // String hashStr = "";

  // for (int i = 0; i < SHA256_SIZE; i++) 
  // {
  //   char hex[3];
  //   sprintf(hex, "%02x", (unsigned char)hash[i]);  // Convertir cada byte a un valor hexadecimal
  //   hashStr += String(hex);
  // }
  // return hashStr;
}
*/
String GenerarHash(const String& user_pass) 
{
  byte binaryHash[SHA256_BLOCK_SIZE];         //  un array de bytes con el hash pero en formato binario
  char result[1 + SHA256_BLOCK_SIZE << 1];    //  un array de char para el hash en formato ASCII

  // Crear una instancia de la clase Sha256
  Sha256* pSha256 = new Sha256();

  //  sha256.reset();  // Reiniciar el objeto SHA256 (equivalente a init())

  // Usar `add()` para agregar los datos (en este caso la contraseña)
  //  sha256.add((const void*)user_pass.c_str(), user_pass.length());

  pSha256->update((const byte*)user_pass.c_str(), user_pass.length());

  // Obtener el hash resultante como un string hexadecimal
  pSha256->final(binaryHash);

  //char* hash = sha256.getHash();  // Obtiene el hash como un string hexadecimal

  delete pSha256;   //  no usaremos mas este objeto...

  // Convertir el hash hexadecimal a un String y retornarlo

  // String hashStr = "";

  //  esto lo que hace es convertir un array binario por ejemplo
  //  +----
  //  |  |
  //  +------
  //  
  for (int i = 0; i < SHA256_BLOCK_SIZE; i++) 
  {
    sprintf(result+i*2, "%02x", (unsigned char)binaryHash[i]);  // Convertir cada byte a un valor hexadecimal y agregarlo en result
    //  hashStr += String(hex);
  }
  // return hashStr;
  //  256 bits representan 32 bytes o sea que entraria en un char[32]
  //
  return result;
}


/*
  Función para mostrar todos los usuarios logueados

  Muestra la info de los usuarios que tenemos en el session cache
  Nota: puede haber tokens expirados que aun no se han purgado

  TODO colocar una variable de modo para que la funcion retorne enseguida o muestre algo por serial

*/
void MostrarUsuariosLogueados() 
{
  if (!Sesiones.empty())
  {
    //  Serial.println("Usuarios logueados:");
    LOG_INFO("*** Usuarios logueados ***");

    for (UserSession& us: Sesiones)
    {
      auto absolutoActual = std::chrono::milliseconds(millis());
      auto deltaT = absolutoActual - us.LastAccessAbsolute;
      
      LOG_INFO("Email: %s", us.Email.c_str());
      LOG_INFO("Nombre: %s", us.Nombre.c_str());
      LOG_INFO("Rol: %s", ToString(us.Rol).c_str());

      //  LOG_INFO("Absoluto actual: %llu", absolutoActual.count());
      //  LOG_INFO("LastAccessAbsolute: %llu", us.LastAccessAbsolute.count());
      //  LOG_INFO("DeltaT: %llu", deltaT.count());
      LOG_INFO("Tiempo de logueado: %llumin", duration_cast<minutes>(deltaT).count());
    }
  }
}


/*
  Verifica que el token se encuentre en el cache y que ademas no haya expirado
  En caso positivo, retorna true y ademas setea sesion con los datos del cache
  En otro caso retorna false y el contenido de sesion es indefinido
*/
bool ValidarSesion(const String& token, const UserSession& sesion) 
{
  //TODO: realizar un chequeo que la Session no expiro

  bool userLoggedIn = false;
  uint64_t tokenNumerico = 0L;

  //  TODO  forward_list
  for (UserSession& sesion: Sesiones)
  {
    if (sesion.Token == tokenNumerico)
    {
      //  validar que el token siga siendo valido
      // if ()
      // {

      // }
      // else
      // {
      //   LOG_DEBUG("--- Token [%s] expirado el usuario se purgara en la siguiente limpieza", token);
      //   return false;
      // }
    }
  }

  // for (size_t i = 0; i < UsuariosLogueados.size(); i++) 
  // {
  //   if (UsuariosLogueados[i].User == sesion.User && UsuariosLogueados[i].Token == token) 
  //   {
  //     //  TODO doble chequear que el token no haya expirado

  //     userLoggedIn = true;  // Si encontramos el usuario con el token correcto
  //                           // El usuario ya está logueado, se devuelve el mismo token
  //     break;
  //   }
  // }
  return false;
}


/*
  Comprueba si se ha enviado header de fecha/hora en el request y en tal caso, si el dato actual de facha/hora es inexacto, 
  lo actualiza con el valor recibido desde la app externa
*/
void TryProcessDatetimeHeader()
{
  if (server.hasHeader(Datetime_Header))
  {
    String dtValue = server.header(Datetime_Header);

    //  ajustar fecha/hora si puede estar desfasada...
    //
    if (!g_tiempo_ajustado)
    {
      //  parsear contenido del header y setear tiempo exacto
      //
      auto tiempo = ParseDateTime(dtValue.c_str());

      if (tiempo)
      {
        LOG_DEBUG("Header date/time valido. Tiempo impreciso ajustado: %s", dtValue.c_str());
        SetDateTime(*tiempo, true);
      }
    }
  }
  else
    LOG_INFO("Request sin header de fecha/hora");
}


/*
  Comprueba si el request posee un token de autenticacion y si ademas corresponde a una sesion establecida y valida
  De ser asi retorna la sesion, caso contrario el "no valor"
  Podemos pasarle como argumento una funcion de policy opcional que va a definir si el usuario esta o no autorizado
  
*/
std::variant<AuthorizationErrors, UserSession>
Authorize(std::function<bool(UserSession)> policy)
{
  if (server.hasHeader(Token_Header))
  {
    std::string tkText {server.header(Token_Header).c_str()};
    uint64_t tkValue = stoull_seguro(tkText);

    if (tkValue != -1)
    {
      //  token universal...
      if (tkValue == Token_Universal)
      {
        LOG_WARNING("Usando Token Universal...retornando sesion generica");
        return GenericSesion;
      }

      //  ahora cambio el objetivo del metodo, que es autorizar el request, este seria el equivalente de incorporar los
      //  datos del usuario en el request
      //
      auto before = Sesiones.before_begin();
      for (auto it = Sesiones.begin(); it != Sesiones.end(); )
      {
        if (it->Token == tkValue)
        {
          //  el usuario estaba logueado...hay que ver si no expiro el token
          if (IsTokenOutdated(*it))
          {
            LOG_INFO("Token coincidente pero expirado!! Quitamos la sesion!!!");
            
            //  eliminamos la sesion expirada
            Sesiones.erase_after(before);

            return AuthorizationErrors::Token_Expired;
          }

          //  luego aplicar la policy con la sesion y si no se cumple tenemos un 403
          //
          if (policy != nullptr && !policy(*it))
          {
            LOG_INFO("Token coincidente pero no tiene autorizacion!!");
            return AuthorizationErrors::No_Autorizado;
          }

          LOG_INFO("Token coincidente!! Reiniciando login time y retornando sesion en cache...");
          
          it->LastAccessAbsolute = std::chrono::milliseconds(millis());

          return *it;
        }
        else
        {
          before = it;
          ++it;
        }
      }
      //  si llegamos aca es porque no encontramos el token en ninguna de las sesiones, entonces casi seguro que el token fue purgado
      //  o esta cacheado desde una sesion anterior en el cliente
      //  
      LOG_INFO("Encontramos el header de token: %s pero no encontramos usuario asociado, seguramente un token cacheado.", 
        tkText.c_str());

      return AuthorizationErrors::Token_Not_Found;
    }
    else
    {
      //  token mal formado, este es un caso rarooooo
      //
      LOG_ERROR("[%s] ==> Token invalido!! Que esta pasando??", tkText.c_str());
      return AuthorizationErrors::Token_Invalid;   //  No autenticado
    }
  }
  else
  {
    LOG_WARNING("Request sin token asociado");
    return AuthorizationErrors::No_Autenticado;   //  No autenticado
  }
}


void PurgarTokensExpirados()
{
  //  recorremos la lista de Usuarios y eliminamos de la lista los tokens que ya han expirado
  //
  Sesiones.remove_if([](UserSession& sesion) {
    bool result = IsTokenOutdated(sesion);

    if (result)
      LOG_INFO("Se va a purgar el token de %s", sesion.Email.c_str());

    return result;
  });
}


/*
  Verifica si el token es muy viejo, permite 
*/
bool IsTokenOutdated(const UserSession& sesion)
{
  auto absolutoActual = std::chrono::milliseconds(millis());
  auto deltaT = duration_cast<minutes>(absolutoActual - sesion.LastAccessAbsolute);

  switch (sesion.Rol)
  {
    case Roles::Admin_Rol:
      if (deltaT > Admin_Token_Expiration_Time)
        return true;
      break;

    case Roles::User_Rol:
      if (deltaT > User_Token_Expiration_Time)
        return true;
      break;
  }
  return false;
}


/// Muestra por serial un listado de datos mas o menos importantes para saber con que tipo de hardware estamos tratando
/// Ejemplo: tamaños de ciertos tipos de datos, ID de la memoria flash, etc
///
void ShowHardwareInfo()
{
  Serial.printf("[[[FLASH]]]\n");
  Serial.printf("Vendor  ID: 0x%02x\n", ESP.getFlashChipVendorId());
  Serial.printf("Chip  ID: 0x%08x\n", ESP.getFlashChipId());
  Serial.printf("Size (bytes): %i\n", ESP.getFlashChipSize());
  Serial.printf("Real Size (bytes): %i\n", ESP.getFlashChipRealSize());
  Serial.printf("Velocidad (Hz): %i\n", ESP.getFlashChipSpeed());

  Serial.printf("sizeof(bool) = %zd\n", sizeof(bool));
  Serial.printf("sizeof(uint8_t) = %zd\n", sizeof(uint8_t));
  Serial.printf("sizeof(uint16_t) = %zd\n", sizeof(uint16_t));
  Serial.printf("sizeof(uint32_t) = %zd\n", sizeof(uint32_t));
  Serial.printf("sizeof(uint64_t) = %zd\n", sizeof(uint64_t));
  Serial.printf("sizeof(unsigned long) = %zd\n", sizeof(unsigned long));
  Serial.printf("sizeof(unsigned long long) = %zd\n", sizeof(unsigned long long));
  
  Serial.printf("sizeof(time_t) = %zd\n", sizeof(time_t));
  Serial.printf("sizeof(milliseconds) = %zd\n", sizeof(milliseconds));
  Serial.printf("sizeof(DiaSemana) = %zd\n", sizeof(DiaSemana));
  Serial.printf("sizeof(Horario) = %zd\n", sizeof(Horario));

  Serial.printf("sizeof(DatabaseConfig) = %zd\n", sizeof(DatabaseConfig));
  Serial.printf("sizeof(PersistentConfig) = %zd\n", sizeof(PersistentConfig));
  Serial.printf("sizeof Configuration = %zd\n", sizeof Configuration);


  Serial.printf("Hash para 12345678: %s\n", GenerarHash("12345678").c_str());

  Serial.printf("EEPROM Start: 0x%08x\n", EEPROM_start);
}

/*
  OJO CON ESTA FUNCION
*/
void FactoryReset()
{
  WiFiManager wm;

  wm.resetSettings();
  Configuration.Erase();

  ESP.restart();
}

/*
  ------------------------------------------------
                  SETUP Y LOOP
  ------------------------------------------------
*/

void setup() 
{
  //  seteamos el loglevel a verbose mientras no estemos en RELEASE
  //  deberiamos guardarlas en EEPROM!!
  //
  g_min_log_level = LogLevel::Information;

  Serial.begin(115200);
  delay(10);
  Serial.printf("\n\n");    //  simplemente para separar del glitch que se recibe al iniciar el chip

  pinMode(Feedback_Pin, OUTPUT);
  digitalWrite(Feedback_Pin, LOW);

  Configuration.Read();

  if (Configuration.HasData())
  {
    if (Configuration.LastTimeSaved > 0)
    {
      LOG_INFO("Recuperando date/time desde EEPROM (inexacto)");
      SetDateTime(Configuration.LastTimeSaved, false);
    }
  }

  //  TaskFeedback
  //
  schedule_recurrent_function_us([](){
    static uint8_t estado = HIGH;
    static uint8_t count = 0;

    //  Serial.printf(".");
    if (g_enable_feedback)
    {
      //  Serial.printf("#");
      if (count == g_feedback_count)
      {
        //  Serial.printf("%% %i", estado);
        count = 0;
        digitalWrite(Feedback_Pin, estado);
        estado = (estado == HIGH) ? LOW : HIGH;
      }
      else
      {
        //  Serial.printf("%i ", count);
        count++;
      }
    }
    return true;
  }, 100000);

  StopWatch sw;
  sw.Start();

  WiFi.mode(WIFI_AP_STA);

  WiFiManager wm;

  wm.setCustomHeadElement("TUSErradura");

  //  seteamos algunos valores por ahora
  wm.setConfigPortalTimeout(60);
  wm.setConnectTimeout(20);

  wm.setAPCallback([](WiFiManager* pwm){
    LOG_INFO("AP Callback");
    g_enable_feedback = true;
    g_feedback_count = Feedback_AccessPoint_Delay;
  });

  if (!wm.getWiFiIsSaved())
  {
    LOG_INFO("No tenemos guardado nada...configuracion de fabrica??");
  }
  else
  {
    //  LOG_INFO("..");
    LOG_INFO("Tenemos guardado un AP %s", wm.getWiFiSSID().c_str());      //  esto funciona OK
    g_enable_feedback = true;
    g_feedback_count = Feedback_WiFiConnect_Delay;
  }

  //wiFiManager.resetSettings();

  //  mostramos algunas cosas del WiFi Manager...
  //  LOG_INFO("WiFi Manager - parameters count: %i", wm.getParametersCount());
  //  LOG_INFO("WiFi Manager - : %s", wm.);
  //  LOG_INFO("WiFi Manager - : %s", wm);
  //  LOG_INFO("WiFi Manager - : %s", wm);


  wm.setConfigPortalTimeoutCallback([](){
    LOG_INFO("Config Portal Timeout Callback");
  });

  // wm.setco ([](WiFiManager* pwm){
  //   LOG_INFO("AP Callback");
  // });

  //  iniciar feedback de intento de conexion...
  
  if (!wm.autoConnect("AutoConnectAP", "12345678")) 
  {
    Serial.println("Fallo en la conexión");
    delay(3000);
    ESP.restart();  // Reiniciar si la conexión falla
  } 
  else 
  {
    g_enable_feedback = false;

    Serial.println();
    Serial.print("Conectado a:\t");
    Serial.println(WiFi.SSID());
    Serial.print("IP address:\t");
    Serial.println(WiFi.localIP());
    Serial.println("Mac Address: /t");
    Serial.println(WiFi.macAddress());
  }

  ShowHardwareInfo();
  
  //  Definir Pin para la interaccion con la cerradura electronica
  //
  pinMode(Door_Pin, OUTPUT);
  digitalWrite(Door_Pin, LOW);

/*
  Roles adminRol = ParseRol("AdmiN");
  Roles userRol = ParseRol("uSEr");

  LOG_INFO("Rol Admin: %i - Rol User: %i\n", (int)adminRol, (int)userRol);
*/

  //  si la base de datos esta vacia agregamos un usuario por defecto que sera administrador
  //  este usuario deberia invalidarse cuando tengamos otro usuario administrador ingresado
  //
  ServiciosDatabase dbServices;

  if (!dbServices.HasUsersRegistered())
  {
    LOG_INFO("DB vacia, agregando usuarios estandard para que se pueda interactuar");

    dbServices.AddUser("default_admin@borrar.ya", "Administrador Temporal", "123456", "admin");
    dbServices.AddUser("default_user@borrar.ya", "Usuario Temporal", "123456", "user");
  }

  Serial.printf("Lapso: %7.2f\n", sw.ElapseFloat().count());

  //  avisamos al server que headers del request nos interesan...
  //
  server.collectHeaders(Datetime_Header, Token_Header);

  //  end-point general de info, ej mac address y otras cosillas
  //
  server.on("/get_info", HandleInfo);

  //  end-point para login
  //
  server.on("/login", HTTP_POST, HandleLogin);

  // Endpoint para Abrir la Puerta
  //
  server.on("/door/open", HandleOpenDoor);

  // Endpoint para agregar un nuevo usuario
  //
  server.on("/add_user", HTTP_POST, HandleAddUser);

  // Endpoint para borrar un usuario
  server.on("/delete_user", HTTP_POST, HandleDeleteUser);

  // Endpoint para resetear la configuracion (EXPERIMENTAL)
  //
  server.on("/reset_fabrica", HTTP_POST, HandleFactoryReset);

  // Mensaje de error
  server.onNotFound([]() {
    server.send(404, "text/plain", "Ruta no encontrada");
  });

  server.begin();

  //
  //
  //

  ScheduleFuntion([](){
    static uint8_t estado = HIGH;
    // static char buffer[50];  //  para mostrar fecha/hora simples

    // //  imprime la fecha y hora cada segundo...
    // //
    // std::time_t t_ticks = time(nullptr);
    // std::tm* t_local = localtime(&t_ticks);  //  ojo este struct esta creado en el sistema
    // std::strftime(buffer, 49, "%c", t_local);
    // Serial.printf("%s - %i\n", buffer, ClearSession_Time);
    
    //Serial.printf("%s\n", GetCurrentDatetime());

    //  experimentos con chrono...
    //
    //  ahora es time_point<system_clock>
    // auto ahora = std::chrono::system_clock::now();
    // std::time_t ahora_t = std::chrono::system_clock::to_time_t(ahora);

    // Serial.printf("time_t --> %lld ; count --> %lld\n", ahora_t, ahora.time_since_epoch().count());

    digitalWrite(Watchdog_Pin, estado);
    estado = (estado == HIGH) ? LOW : HIGH;

    return true;
  }, 1s);
  
  ScheduleFuntion([](){
    // Mostrar los usuarios logueados cada cierto tiempo
    // mostrarUsuariosLogueados();
    LOG_VERBOSE("Mostrar los usuarios logueados cada cierto tiempo");

    //  GenerarToken();

    MostrarUsuariosLogueados();
    return true;
  }, Watchdog_Time);

  ScheduleFuntion([](){
    LOG_INFO("Purgar usuarios del cache cuyo token haya expirado");

    // // Verificar si la sesión ha expirado (15 minutos)
  
    // if (millis() - lastLoginTime > sessionLimit) {
    //   UsuariosLogueados.clear();  // Limpiar los usuarios logueados después de 15 minutos
    //   Serial.println("Sesión expirada, usuarios desconectados.");
    // }

    //  UsuariosLogueados.clear();  // Limpiar los usuarios logueados después de 15 minutos
    PurgarTokensExpirados();

    //  Serial.println("Sesión expirada, usuarios desconectados.");
    
    return true;
  }, Purge_Tokens_Time);

}


void loop() 
{
  server.handleClient();  // Mantenemos el servidor funcionando
  //  delay(10000);  // Muestra los usuarios cada 10 segundos
  //  EEPROM.begin(100);
  yield();
}


/*
  if (server.hasArg("user") && server.hasArg("pass")) {
    String User = server.arg("user");
    String Pass = server.arg("pass");
    String newRol = server.arg("nweRol");
    String newUser = server.arg("newUser");
    String newPass = server.arg("newPass");

    //verificar si la sesion es valida

    if(ValidarSesion(User,Token){
        //si tengo sesion me fijo que sea Admin

        if (EsAdmin(User,Token)){
              // Si el usuario no existe, agregamos el nuevo usuario
              User newUser;
              newUser.User = newUser;
              newUser.PassHash = generarHash(newPass); // Guardamos el hash de la contraseña
              newUser.Rol = newRol;
              if(SaveNewUser){
                   server.send(200, "text/plain", "Usuario agregado exitosamente.");
              }else{
                  server.send(400, "text/plain", "Erro(1005): Usario no agregado")
                }
              }
        }
  } else {
    server.send(400, "text/plain", "Error: Parámetros faltantes (user, pass, rol).");
  }
  */

/*bool EsAdmin(String User, String Token){
     bool EsAdmin = false;
    for (size_t i = 0; i < UsuariosLogueados.size(); i++) {
      if (UsuariosLogueados[i].User == User && UsuariosLogueados[i].Token == Token) {
        if(UsuariosLogueados[i].Rol == "Admin"){
            EsAdmin = true;
            break;
        }  // Si encontramos el usuario con el token correcto
        
      }
    }
    return EsAdmin; 
}*/
