
#include "repositorios.h"
#include <cstring>

UsuarioDTO* 
RepositorioUsuarios::getUserFromId(std::string id)
{
  return nullptr;
}


bool
RepositorioUsuarios::addUser(std::string id, std::string nombre, std::string pass, std::string rol)
{
  FileDriver driver;

  DBUsuario agregar;

  strcpy(agregar.nombre, nombre.c_str());

  return driver.addUsuario(agregar);
}

