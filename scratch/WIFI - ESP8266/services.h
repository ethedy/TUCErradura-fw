
#ifndef SERVICES_H
#define SERVICES_H

//  lamentablemente tengo que usar esta libreria de cuarta y no std::string
#include <WString.h>
//  #include "c_types.h"
#include <cstdint>


/*
  struct para guardar la info de un un usuario que tiene una sesion establecida en la cerradura

  Renombramos 'Session' a 'UserSession' para evitar el conflicto
*/
struct UserSession {
  String User;
  String PassHash;     // Usaremos el hash de la contraseña
  String Rol;          // "admin" o "user"
  String Token;        // Token de sesión único para cada usuario
  long lastLogin;     // Token de sesión único para cada usuario
  String Status;          //  texto del error, o bien OK
  uint16_t StatusCode;    //  0 => no error, de otra manera el codigo del error

  //  tal vez tengamos que agregar ctor y otros metodos
};

class SecurityServices
{
  public:
    UserSession login_user(const String& user_name, const String& pass);

};

#endif
