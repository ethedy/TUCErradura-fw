
#ifndef REPOSITORIOS_H
#define REPOSITORIOS_H

#include "driver_file.h"
#include <string>

struct UsuarioDTO 
{
  std::string Nombre;
  std::string Password;
  std::string Rol;
  std::string Mail;
}

class RepositorioUsuarios 
{
public:
  UsuarioDTO* getUserFromId(const std::string& id):

  bool addUser(const std::string& id, const std::string& nombre, const std::string& pass, const std::string& rol);
};


#endif