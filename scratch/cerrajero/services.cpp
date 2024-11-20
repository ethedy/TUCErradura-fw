
#include "services.h"
#include "repositorios.h"
#include <string>

using namespace std;

UserSession
SecurityServices::login_user(const String& user_name, const String& pass) 
{
  UserSession result;

  //  algunas validaciones para usuario y pass
  //
  if (
    user_name == nullptr || 
    user_name.trim().length() == 0 ||
    pass == nullptr ||
    pass.trim().length() == 0)
  {
    result.StatusCode = -100;
    result.Status = "Nombre de usuario o contraseña vacios o nulos";
  }
  //
  //  instanciar un repositorio e invocar al metodo correspondiente
  //
  RepositorioUsuarios userRepo;

  UsuarioDTO* usuario = userRepo.getUserFromId(user_name.c_str()); 

  if (usuario != nullptr)
  {
    if (usuario->Password == pass.c_str())
    {
      //  correcto!!!
      //
      result.User = user_name;
      result.Rol = usuario->Rol;    //  ver casting
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
    result.StatusCode = -200;
    result.Status = "El usuario no existe";
  }
}


bool 
SecurityServices::add_user(const String& id, const String& user_name, 
                           const String& pass, const String& rol)
{
  RepositorioUsuarios userRepo;

  if (!userRepo.addUser())
  {
    //  deberiamos poner algun mensaje
    return false;
  }
  return true;
}

