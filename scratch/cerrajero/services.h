
#ifndef SERVICES_H
#define SERVICES_H

#include <string>

/*
  struct para guardar la info de un un usuario que tiene una sesion establecida en la cerradura

  Renombramos 'Session' a 'UserSession' para evitar el conflicto
*/
struct UserSession {
  std::string User;
  std::string PassHash;     // Usaremos el hash de la contraseña
  std::string Rol;          // "admin" o "user"
  std::string Token;        // Token de sesión único para cada usuario

  //  tal vez tengamos que agregar ctor y otros metodos
};




#endif