/*
  CERRAJERO.INO

  Autor: Enrique Thedy
  Fecha: NOV-DIC/2024
  Version: 1

  Maqueta de uso de base de datos para el sistema de cerradura electronica de TUSA

*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
//  #include <sha256.h> // Incluir la clase sha256.h que has proporcionado
//  #include <WString.h>
//  #include "wifi_selector.h"
//  #include <esp_system.h>
#include <Arduino.h>
#include <pins_arduino.h>
#include <ets_sys.h>
#include <gpio.h>
#include <Schedule.h>
#include <HardwareSerial.h>
#include <time.h>
#include <sys/time.h>

#include "Utiles.h"
#include "Servicios.h"
#include "Entidades.h"

#include <vector>  
#include <algorithm>
#include <string>
//  #include <regex>
#include <chrono>


using namespace Servicios;
using namespace Entidades;

#define SHA256_SIZE 32 // El tamaño del hash SHA256 es 32 bytes

ESP8266WebServer server(80);

unsigned long sessionLimit = 900000;    // 15 minutos de tiempo para session activa
unsigned long lastLoginTime = 0;        // Para gestionar la expiración de la sesión

const int Puerta = 2;                   // Pin del LED

const uint8_t  Watchdog_Pin = 2;
const uint32_t Watchdog_Time =  1 * 1000000;

const char* Datetime_Header = "X-Tuse-Datetime";
const char* Token_Header = "X-Tuse-Token";

//ETSEvent cola[10];

std::vector<Servicios::UserSession> UsuariosLogueados;  // Vector para almacenar los usuarios logueados


/*
*/
void TryProcessDatetimeHeader();

String TryGetTokenFromRequest();


// Función para generar un token único
String generarToken() {
  String token = String(millis(), HEX) + String(random(0, 255), HEX);  // Usamos el tiempo y un valor aleatorio como token
  return token;
}


// Función para generar el hash SHA256 de la contraseña utilizando la clase `SHA256` proporcionada
// String generarHash(const String& pass) {
//   SHA256 sha256;  // Crear un objeto de la clase SHA256
//   sha256.reset(); // Reiniciar el objeto SHA256 (equivalente a init())

//   // Usar `add()` para agregar los datos (en este caso la contraseña)
//   sha256.add((const void*)pass.c_str(), pass.length());

//   // Obtener el hash resultante como un string hexadecimal
//   char* hash = sha256.getHash();  // Obtiene el hash como un string hexadecimal

//   // Convertir el hash hexadecimal a un String y retornarlo
//   String hashStr = "";
//   for (int i = 0; i < SHA256_SIZE; i++) {
//     char hex[3];
//     sprintf(hex, "%02x", (unsigned char)hash[i]);  // Convertir cada byte a un valor hexadecimal
//     hashStr += String(hex);
//   }
//   return hashStr;
// }

/*
*/

//   Verificar usuario con hash de contraseña
//
bool verificarUsuario(const String& User, const String& Pass, UserSession& usuario) {
  for (size_t i = 0; i < UsuariosLogueados.size(); i++) {
    if (UsuariosLogueados[i].User == User.c_str()) {
      // Comparamos el hash de la contraseña
      //if (UsuariosLogueados[i].PassHash == generarHash(Pass)) {
        usuario = UsuariosLogueados[i]; // Devolvemos la información del usuario encontrado
        return true;
      //}
    }
  }
  return false;
}

bool Validarsesion(String Token, UserSession& u){

  //TODO: realizar un chequeo que la Session no expiro

  bool userLoggedIn = false;

  for (size_t i = 0; i < UsuariosLogueados.size(); i++) 
  {
    if (UsuariosLogueados[i].Token == Token) 
    {
      userLoggedIn = true;  // Si encontramos el usuario con el token correcto
        // El usuario ya está logueado, se devuelve el mismo token
      break;
    }
  }
  return userLoggedIn; 
}


// Función para manejar la solicitud de login
void handleLogin() {
  TryProcessDatetimeHeader();   //  siempre tratamos de aprovechar el header de datetime

  UserSession usuario; 

  //TODO: verificar que el usuario se encuentra logueado 

  if(server.hasArg("Token"))
  {
    if(Validarsesion(server.arg("Token"), usuario)){
      server.send(200, "text/plain", "Usuario ya conectado.");
      return;
    }
  }

  if (server.hasArg("user") && server.hasArg("pass")) 
  {
    String User = server.arg("user");
    String Pass = server.arg("pass");
    ServiciosSeguridad Validacion;

    usuario = Validacion.LoginUser(User, Pass);

    if (usuario.StatusCode == 0) 
    {
      // Generamos un nuevo token para este usuario y lo añadimos
      usuario.Token = generarToken();
      usuario.LastLogin = millis(); // Marcar el tiempo del último login
      // Si es válido, lo agregamos al vector de usuarios logueados
      UsuariosLogueados.push_back(usuario);

      server.send(200, "text/plain", "Login exitoso. Token: " + usuario.Token);
    } 
    else 
    {
      // Si no es válido, mostramos un mensaje de error
      server.send(401, "text/plain", "Error: Usuario o contraseña incorrectos.");
    }
  } 
  else 
  {
    server.send(400, "text/plain", "Error: Parámetros de usuario y contraseña faltantes.");
  }
}

// Lógica para abrir la puerta
void handleOpenDoor() {
  TryProcessDatetimeHeader();   //  siempre tratamos de aprovechar el header de datetime  

  if (server.hasArg("user") && server.hasArg("token")) {
    String User = server.arg("user");  // Recuperamos el usuario desde los parámetros del POST
    String Token = server.arg("token"); // Recuperamos el token desde los parámetros del POST

    bool userLoggedIn = false;
    for (size_t i = 0; i < UsuariosLogueados.size(); i++) {
      if (UsuariosLogueados[i].User == User.c_str() && UsuariosLogueados[i].Token == Token.c_str()) {
        userLoggedIn = true;  // Si encontramos el usuario con el token correcto
        break;
      }
    }

    if (userLoggedIn) {
      // El usuario está logueado, proceder a abrir la puerta
      digitalWrite(Puerta, LOW);  // Abre la puerta
      server.send(200, "text/plain", "Puerta abierta exitosamente.");
      delay(5000);  // Mantiene la puerta abierta durante 5 segundos
      digitalWrite(Puerta, HIGH);  // Cierra la puerta
    } else {
      // El usuario no está logueado, mostramos un mensaje de error
      server.send(401, "text/plain", "Error: Usuario no autenticado.");
    }
  } else {
    // Si no se recibe el parámetro 'user' o 'token', mostramos un mensaje de error
    server.send(400, "text/plain", "Error: Parámetros 'user' o 'token' faltantes.");
  }
}

// Función para agregar un nuevo usuario
void handleAddUser() 
{
  TryProcessDatetimeHeader();   //  siempre tratamos de aprovechar el header de datetime

  if (server.hasArg("user") && server.hasArg("pass") && server.hasArg("name") && server.hasArg("rol")) {
    String newUser = server.arg("user");
    String newPass = server.arg("pass");
    String newRol = server.arg("rol");

    // Verificar si el usuario ya existe
    for (size_t i = 0; i < UsuariosLogueados.size(); i++) {
      if (UsuariosLogueados[i].User == newUser.c_str()) {
        server.send(400, "text/plain", "Error: El usuario ya existe.");
        return;
      }
    }

    // Si el usuario no existe, agregamos el nuevo usuario
    UserSession newSession;
    newSession.User = newUser.c_str();
    //newSession.PassHash = generarHash(newPass); // Guardamos el hash de la contraseña
    newSession.Rol = newRol.c_str();

    UsuariosLogueados.push_back(newSession);
    server.send(200, "text/plain", "Usuario agregado exitosamente.");
  } else {
    server.send(400, "text/plain", "Error: Parámetros faltantes (user, pass, rol).");
  }
}

// Función para eliminar un usuario
void handleDeleteUser() 
{
  TryProcessDatetimeHeader();   //  siempre tratamos de aprovechar el header de datetime

  if (server.hasArg("user")) {
    String userToDelete = server.arg("user");

    // Buscar el usuario en el vector
    for (size_t i = 0; i < UsuariosLogueados.size(); i++) {
      if (UsuariosLogueados[i].User == userToDelete.c_str()) {
        // Eliminar el usuario encontrado
        UsuariosLogueados.erase(UsuariosLogueados.begin() + i);

        server.send(200, "text/plain", "Usuario eliminado exitosamente.");
        return;
      }
    }

    // Si el usuario no fue encontrado
    server.send(404, "text/plain", "Error: Usuario no encontrado.");
  } else {
    server.send(400, "text/plain", "Error: Parámetro 'user' faltante.");
  }
}

// Función para mostrar todos los usuarios logueados
void mostrarUsuariosLogueados() {
  Serial.println("Usuarios logueados:");
  for (size_t i = 0; i < UsuariosLogueados.size(); i++) {
    Serial.print("Usuario: ");
    Serial.print(UsuariosLogueados[i].User.c_str());
    Serial.print(", Rol: ");
    Serial.println(UsuariosLogueados[i].Rol.c_str());
  }
}


/*
*/
void TryProcessDatetimeHeader()
{
  if (server.hasHeader(Datetime_Header))
  {
    String dtValue = server.header(Datetime_Header);

    LOG_DEBUG("Encontramos el header de fecha/hora: %s", dtValue.c_str());
  }
  else
    LOG_WARNING("Request sin header de fecha/hora");
}


void setup() {
  g_min_log_level = LogLevel::Verbose;

  Serial.begin(115200);
  delay(10);

  WiFi.mode(WIFI_STA);
  WiFi.begin("MUTEX", "sqlSDK@1967");
  
  while (WiFi.status() != WL_CONNECTED) 
  {
    esp_delay(500);
    Serial.print(".");
  }
  Serial.printf("\nConectado con MUTEX!!!\nDireccion IP: %s %s\n", WiFi.localIP().toString().c_str(), WiFi.getHostname());

  auto i = sizeof(__func__);
  
  char pirulo[100] ;

  Serial.printf("Tamaño: %zd\n", i);
  
  strcpy(pirulo, __func__);

  Serial.printf("Nombre: %s\n", pirulo);

  //  LogTest(__func__);
  //  const char* pp = __func__;
  //  DumpMemory(pp, 16, "Desde setup ");

  //  Log(__func__, LogLevel::Warning, "Valor %i\n", 12);
  LOG_WARNING("Valor %i", 12);

  std::string brian {"br.ian+brian-brian"};
  std::regex emailNameValidator{R"(^([a-zA-Z0-9+-]+(\.[a-zA-Z0-9+-]+)*)$)"};

  if (std::regex_match(brian, emailNameValidator))
  {
    printf("Valido!!\n");
  }
  else
    printf("NOOOOOOOO Valido!!\n");

  // try
  // {
  // }
  // catch (std::exception ex)
  // {
  //   Serial.printf("Exception: %s\n", ex.what());
  // }
  
  
  //  WiFiManager wiFiManager;
  //  wiFiManager.resetSettings();
  // if (!wiFiManager.autoConnect("MUTEX", "sqlSDK@1967")) {
  //   Serial.println("Fallo en la conexión");
  //   delay(3000);
  //   ESP.restart(); // Reiniciar si la conexión falla
  // } else {
  //   Serial.println();
  //   Serial.print("Conectado a:\t");
  //   Serial.println(WiFi.SSID());
  //   Serial.print("IP address:\t");
  //   Serial.println(WiFi.localIP());
  //   Serial.println("Mac Address: /t");
  //   Serial.println(WiFi.macAddress());
  // }

  // Definir Pin para verificar solicitud HTTP
  pinMode(Puerta, OUTPUT);
  digitalWrite(Puerta, LOW);

  server.collectHeaders(Datetime_Header, Token_Header);

  server.on("/get_mac", []() {
    String mac = WiFi.macAddress();
    server.send(200, "text/plain", mac);
  });

  // Endpoint para login
  server.on("/login", HTTP_POST, handleLogin);

  // Endpoint para Abrir la Puerta
  server.on("/door/open", handleOpenDoor);

  // Endpoint para agregar un nuevo usuario
  server.on("/add_user", HTTP_POST, handleAddUser);

  // Endpoint para borrar un usuario
  server.on("/delete_user", HTTP_POST, handleDeleteUser);

  // Mensaje de error
  server.onNotFound([]() {
    server.send(404, "text/plain", "Ruta no encontrada");
  });

  server.begin();

  //  seteamos una fecha/hora totalmente arbitraria
  std::tm settime {
    .tm_sec = 0, .tm_min = 44, .tm_hour = 2,
    .tm_mday = 3, .tm_mon = 0, .tm_year = 2025 - 1900
  };
  
  time_t ticks = mktime(&settime);

  timeval tv {
    .tv_sec = ticks,
    .tv_usec = 0
  };

  settimeofday(&tv, nullptr);

  //  ets_task(mi_tarea, 2, cola, 10);

  /*
    Watchdog para indicar que estamos bien...todos los sistemas funcionando...  
  
  */
  schedule_recurrent_function_us([]() 
    {
      static uint8_t estado = HIGH;
      static char buffer[50];   //  para mostrar fecha/hora simples

      //  imprime la fecha y hora cada segundo...
      //      
      // std::time_t t_ticks = time(nullptr);
      // std::tm* t_local = localtime(&t_ticks) ;    //  ojo este struct esta creado en el sistema
      // std::strftime(buffer, 49, "%c", t_local);
      // Serial.printf("%s\n", buffer);

      //  experimentos con chrono...
      //
      //  ahora es time_point<system_clock>
      // auto ahora = std::chrono::system_clock::now();
      // std::time_t ahora_t = std::chrono::system_clock::to_time_t(ahora);
      
      // Serial.printf("time_t --> %lld ; count --> %lld\n", ahora_t, ahora.time_since_epoch().count());

      digitalWrite(Watchdog_Pin, estado);

      //Serial.printf("Hola aca estoy...Estado = %i\n", estado);

      estado = (estado == HIGH) ? LOW : HIGH;

      return true;
    }, Watchdog_Time);

  DiaSemana dia = Lunes;

  Serial.printf("sizeof(uint8_t) = %zd\n", sizeof(uint8_t));
  Serial.printf("sizeof(uint16_t) = %zd\n", sizeof(uint16_t));
  Serial.printf("sizeof(uint32_t) = %zd\n", sizeof(uint32_t));
  Serial.printf("sizeof(DiaSemana) = %zd\n", sizeof(DiaSemana));
  Serial.printf("sizeof(Horario) = %zd\n", sizeof(Horario));
}

void loop() {
  server.handleClient(); // Mantenemos el servidor funcionando

  // Mostrar los usuarios logueados cada cierto tiempo
  //mostrarUsuariosLogueados();

  // Verificar si la sesión ha expirado (15 minutos)

  // if (millis() - lastLoginTime > sessionLimit) {
  //   UsuariosLogueados.clear();  // Limpiar los usuarios logueados después de 15 minutos
  //   Serial.println("Sesión expirada, usuarios desconectados.");
  // }

  //  ojo con este delay que me esta haciendo lento todo el sistema!!
  //
  //delay(10000);  // Muestra los usuarios cada 10 segundos
  //  ets_post(2, 0, 0);

}

//  probamos si la funcionalidad de task se puede usar...
//

// void mi_tarea(ETSEvent* eventos)
// {
//   Serial.printf("Hola aca estoy...\n");
//   //  esp_schedule();
//   //esp_suspend();
//   esp_delay(2000);
//   esp_yield();
//   //  ets_post(1, 0, 0);
// }
