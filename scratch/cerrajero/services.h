
#ifndef SERVICES_H
#define SERVICES_H

//  lamentablemente tengo que usar esta libreria de cuarta y no std::string
#include <WString.h>
//  #include "c_types.h"
#include <cstdint>

// TODO cambiar id por email
// nombre de usuario es user
// 
/*
  struct para guardar la info de un un usuario que tiene una sesion establecida en la cerradura

  Renombramos 'Session' a 'UserSession' para evitar el conflicto
*/
struct UserSession 
{
  String Email;     //  correo (ID)
  String User;      //  nombre del usuario
  //  String PassHash;          //  Usaremos el hash de la contraseña
  String Rol;               //  "admin" o "user"
  String Token;             //  Token de sesión único para cada usuario
  uint64_t LastLogin;       //  valor de milisegundos que representa el tiempo de login, NO ES fecha/hora
  //
  String Status;          //  texto del error, o bien OK
  uint16_t StatusCode;    //  0 => no error, de otra manera el codigo del error
  //
  //  tal vez tengamos que agregar ctor y otros metodos
};

struct Horario {
  int dia;    //  0 domingo
  int hora_inicio;
  int minuto_inicio;
  int hora_fin;
  int minuto_fin;
};

class SecurityServices
{
  public:
    UserSession login_user(const String& user_email, const String& pass);

    bool add_user(const String& user_email, const String& user_name, const String& pass, const String& rol);

    bool add_horario(const String&);
};

#endif
