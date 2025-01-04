/*
    SERVICIOS
 
    - ServiciosSeguridad:
    - ServiciosDatabase:
    - ServiciosValidacion:
 
 */

#include "Servicios.h"
#include "Repositorios.h"
#include "Utiles.h"

#include <string>
#include <memory>
#include <chrono>
#include <regex>


namespace Servicios
{
  /*
   *  ------------------------------------------------
   *                  ServiciosSeguridad
   *  ------------------------------------------------
   */

  UserSession
  ServiciosSeguridad::LoginUser(const String& user_email, const String& user_pass)
  {
    UserSession result;
    ServiciosValidacion validar;

    std::string tmp_email = AllTrim(user_email.c_str());
    std::string tmp_pass = AllTrim(user_email.c_str());

    //  algunas validaciones para email y pass
    //
    if (!validar.ValidarEmail(tmp_email) || !validar.ValidarPassword(tmp_pass))
    {
      result.StatusCode = -100;
      result.Status = "El email del usuario o la contraseña tiene valores invalidos";
    }
    else
    {
      //
      //  instanciar database service e invocar al metodo correspondiente
      //
      ServiciosDatabase db;

      auto resultado = db.GetUserByEmail(tmp_email);

      if (resultado)
      {
        if (resultado->Password == user_pass.c_str())
        {
          //  correcto!!!
          //
          result.Email = resultado->Email.c_str();
          result.User = resultado->Nombre.c_str();
          result.Rol = resultado->Rol.c_str();

          result.StatusCode = 0;
          result.Status = "OK";
        }
        else
        {
          result.StatusCode = -300;
          result.Status = "Credenciales invalidas!!";
        }
      }
      else
      {
        //  LOG_WARNING("unique_ptr no contiene un objeto, sorry gordo...");
        LOG_WARNING("optional<Usuario> no retorno un objeto...sorry gorda...");

        result.StatusCode = -200;
        result.Status = "El usuario no existe";
      }
    }

    return result;
  }

  /*
   *  ------------------------------------------------
   *                  ServiciosDatabase
   *  ------------------------------------------------
   */


  bool
  ServiciosDatabase::AddUser(const String& user_email, const String& user_name, const String& user_pass,
                             const String& user_rol)
  {
    RepositorioUsuarios userRepo;
    ServiciosValidacion validar;

    std::string tmp_email = AllTrim(user_email.c_str());
    std::string tmp_name = AllTrim(user_name.c_str());
    std::string tmp_pass = AllTrim(user_pass.c_str());
    std::string tmp_rol = AllTrim(user_rol.c_str());

    if (validar.ValidarEmail(tmp_email) &&
        validar.ValidarPassword(tmp_pass) &&
        validar.ValidarNombre(tmp_name) &&
        validar.ValidarRol(tmp_rol))
    {
      return userRepo.AddUser(tmp_email, tmp_name, tmp_pass, tmp_rol);
    }

    LOG_WARNING("No se pudo ingresar el usuario en la DB, revisar log de errores");

    return false;
  }

  std::optional<Usuario>
  ServiciosDatabase::GetUserByEmail(const std::string& user_email)
  {
    RepositorioUsuarios userRepo;

    std::optional<Usuario> result;
    std::string tmp_user = AllTrim(user_email);

    auto temp = userRepo.GetUserFromId(tmp_user);

    if (temp.second)
      result = temp.first;

    return result;
  }

  std::optional<Usuario>
  ServiciosDatabase::GetUserByEmail(const String& user_email)
  {
    std::string email {user_email.c_str()};
    return GetUserByEmail(email);
  }


  /*
   *  ------------------------------------------------
   *                  ServiciosValidacion
   *  ------------------------------------------------
   */

  //  podriamos hacer un parse? o un ctor?
  //  un dia por vez...
  //  es mas probable que la habilitacion sea Lun y Mie por ejemplo y no los 5 dias
  //
  Horario ServiciosValidacion::BuildHorario(const String& dia, const String& hora_inicio, const String& hora_fin)
  {
    Horario nuevo;

    return nuevo;
  }

  //  validar que no este vacio o sean todos espacios
  //  valido que la longitud total sea un minimo para evitar el a@b.c
  //  ademas debe contener un @ y al menos un caracter . luego de la @
  //  o usar expresiones regulares y listo --> pag 841 ProfC++
  //
  bool ServiciosValidacion::ValidarEmail(const std::string& email)
  {
    if (IsNullOrWhitespace(email))
    {
      LOG_WARNING("El email esta vacio o tiene solo espacios");
      return false;
    }
    //  hacer el trim() para chequear realmente la parte que interesa
    //
    std::string check = AllTrim(email);

    if (check.length() < MIN_EMAIL_LENGTH)
    {
      LOG_WARNING("La longitud del email es insuficiente. Minimo: %i ; Actual: %i", MIN_EMAIL_LENGTH, check.length());
      return false;
    }
    if (check.length() > MAX_EMAIL_LENGTH)
    {
      LOG_WARNING("La longitud del email excede el maximo. Maximo: %i ; Actual: %i", MAX_EMAIL_LENGTH, check.length());
      return false;
    }

    //  por ultimo validamos que sea un correo electronico minimamente valido...
    //  para los dominios validamos <dominio>.<TLD> o bien <dominio>.<TLD>.<ccTLD>
    //
    //  Problemas de stack: para validar nombres muy largos
    //  std::regex emailValidator{R"(^([a-zA-Z0-9!#$%&'*+/=?^_`{|}~-]+(\.[a-zA-Z0-9!#$%&'*+/=?^_`{|}~-]+)*)@[a-zA-Z0-9]+(-[a-zA-Z0-9]+)*(?:\.[a-zA-Z]{2,})$)"};
    // if (!std::regex_match(check, emailValidator))
    // {
    //   LOG_WARNING("La direccion de email pasada (%s) no se corresponde con el standard valido. Probar algo como nombre@dominio.com", check.c_str());
    //   return false;
    // }
    

    //  partimos la supuesta direccion en dos partes, separadas por un @
    //  si no tenemos @ => direccion invalida
    auto posArroba = check.find('@');

    if (posArroba == std::string::npos)
    {
      LOG_WARNING("La direccion de mail debe tener un @ (%s)", check.c_str());
      return false;
    }

    std::string parte = check.substr(0, posArroba);

    if (ValidarEmailNombre(parte)) 
    {
      parte = check.substr(posArroba+1);
      if (ValidarEmailDominio(parte))
        return true;
    }

    return false;
  }

  //  validar que no este vacio o sean todos espacios
  //
  bool ServiciosValidacion::ValidarNombre(const std::string& nombre)
  {
    if (IsNullOrWhitespace(nombre))
    {
      LOG_WARNING("El nombre no puede estar vacio o consistir unicamente de espacios");
      return false;
    }
    std::string check = AllTrim(nombre);

    if (check.length() < MIN_NOMBRE_LENGTH)
    {
      LOG_WARNING("La longitud del nombre es insuficiente. Minimo: %i ; Actual: %i", MIN_NOMBRE_LENGTH, check.length());
      return false;
    }
    if (check.length() > MAX_NOMBRE_LENGTH)
    {
      LOG_WARNING("La longitud del nombre excede el maximo permitido. Maximo: %i ; Actual: %i", MAX_NOMBRE_LENGTH, check.length());
      return false;
    }

    return true;
  }

  //  validar que no este vacio o sean todos espacios (es lo unico que podemos)
  //  al estar hashed no podemos comprobar tamaños, debe hacerse en el front
  //
  bool ServiciosValidacion::ValidarPassword(const std::string& pass)
  {
    if (IsNullOrWhitespace(pass))
    {
      LOG_WARNING("La password esta vacia o tiene solo espacios");
      return false;
    }

    return true;
  }

  //  validar que no este vacio o sean todos espacios
  //  user/admin son los unicos permitidos por ahora, hacerlo case insensitive pero lo guardamos en minusculas
  //
  bool ServiciosValidacion::ValidarRol(const std::string& rol)
  {
    std::regex rx_roles{"^user|admin$", std::regex::icase};
    //  std::string check = AllTrim(rol);

    //  LOG_WARNING("La password esta vacia o tiene solo espacios");
    if (!std::regex_match(rol, rx_roles))
    {
      LOG_WARNING("Los roles deben ser user o admin, pero se esta pasando el valor %s", rol.c_str());
      return false;
    }
    return true;
  }

  bool ServiciosValidacion::ValidarFecha()
  {
    return true;
  }

  bool ServiciosValidacion::ValidarHora()
  {
    return true;
  }


  bool ServiciosValidacion::IsNullOrWhitespace(const std::string& value)
  {
    std::regex rx_vacio{"[[:s:]]*"};

    return std::regex_match(value, rx_vacio);
  }

  bool ServiciosValidacion::ValidarEmailNombre(const std::string& nombre)
  {
    // if (nombre.size() > MAX_EMAIL_NAME_LENGTH)
    // {
    //   LOG_ERROR("Por razones de STACK el nombre de la direccion de email pasada (%s) no puede superar los %i caracteres", 
    //     nombre.c_str(), MAX_EMAIL_NAME_LENGTH);
    //   return false;
    // }
    //  regex emailNameValidator{R"(^([a-zA-Z0-9!#$%&'*+/=?^_`{|}~-]+(\.[a-zA-Z0-9!#$%&'*+/=?^_`{|}~-]+)*)$)"};
    //
    //  no empieza con punto
    //  no termina con punto
    //  no tiene dos puntos consecutivos
    //
    if (nombre.front() == '.' || nombre.back() == '.' || nombre.find("..") != std::string::npos)
    {
      LOG_WARNING("El nombre [%s] empieza o termina con punto (.) o contiene dos puntos (..) consecutivos", nombre.c_str());
      return false;
    }
    //  solo contiene caracteres validos: a-zA-Z0-9$%&+_-
    //
    const char validos[] = "$%&+_-.";

    for (char c: nombre)
    {
      //  isalnum() --> a-zA-Z0-9
      //  $%&+_-.
      if (!(isalnum(c) || strchr(validos, c) != nullptr))
      {
        LOG_WARNING("El nombre [%s] no contiene caracteres validos (alfanumericos o $%&+_-.)", nombre.c_str());
        return false;
      }
    }
    return true;
  }

  bool ServiciosValidacion::ValidarEmailDominio(const std::string& dominio)
  {
    bool result = false;

    //  regex emailDomainValidator{R"(^[a-zA-Z0-9]+(-[a-zA-Z0-9]+)*(?:\.[a-zA-Z]{2,})+$)"};
    //
    //  no empieza con punto
    //  no termina con punto
    //  no tiene dos puntos consecutivos
    //
    if (dominio.front() == '.' || dominio.back() == '.' || dominio.find("..") != std::string::npos)
    {
      LOG_WARNING("El dominio [%s] empieza o termina con punto (.) o contiene dos puntos (..) consecutivos", dominio.c_str());
      return result;
    }
    //  debe contener al menos un punto...o sea al menos dos fragmentos
    //
    auto posPunto = dominio.find('.');
    int pos = 0;

    //  el primer fragmento es especial porque puede no tiene limite inferior y porque puede contener guiones (no seguidos)
    //
    std::string fragmento = dominio.substr(pos, posPunto);

    if (fragmento.front() == '-' || fragmento.back() == '-' || fragmento.find("--") != std::string::npos)
    {
      LOG_WARNING("El frag1 del dominio [%s] empieza o termina con guion (-) o contiene dos guiones (--) consecutivos\n", dominio.c_str());
      return false;
    }
    for (char c: fragmento)
    {
      //  isalnum() --> a-zA-Z0-9
      //  -
      if (!(isalnum(c) || c == '-'))
      {
        LOG_WARNING("El frag1 del dominio [%s] no contiene caracteres validos (alfanumericos o -)\n", dominio.c_str());
        return false;
      }
    }
    //  resto de los fragmentos, solo caracteres alfanumericos y minimo tamaño de 2
    //
    bool exitDo = false;
    do
    {
      //  buscamos el siguiente punto, primero guardamos la posicion del punto previo
      //
      pos = posPunto+1;
      posPunto = dominio.find('.', pos);

      if (posPunto != std::string::npos)
        fragmento = dominio.substr(pos, posPunto-pos);
      else
      {
        fragmento = dominio.substr(pos);
        exitDo = true;
      }
      if (fragmento.size() < 2)
      {
        LOG_WARNING("El frag2+ del dominio [%s] es menor a 2 caracteres", dominio.c_str());
      }
      for (char c: fragmento)
      {
        if (!isalnum(c))
          LOG_WARNING("El frag2+ del dominio [%s] no contiene caracteres alfanumericos validos", dominio.c_str());
      }
    } while (!exitDo);

    result =true;

    return result;
  }

}
