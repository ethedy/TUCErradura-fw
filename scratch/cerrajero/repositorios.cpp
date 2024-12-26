/*
 *
 * 
 */

#include "Repositorios.h"
#include "Entidades.h"
#include "Utiles.h"

#include <algorithm>

namespace Datos
{
  using namespace std;
  using namespace Entidades;

  std::pair<Usuario, bool>
  RepositorioUsuarios::GetUserFromId(const std::string& id) const
  {
    //  unique_ptr<UsuarioDTO> user = nullptr;
    std::pair<Usuario, bool> result = { {}, false };

    LOG_INFO("Contador: %i", m_ctx.GetContador());

    auto user = m_ctx.Usuarios.Find(id);

    if (user)
    {
      result.first = user.value();
      result.second = true;
    }

    return result;
  }


  bool
  RepositorioUsuarios::AddUser(const std::string& id, const std::string& nombre, const std::string& pass, const std::string& rol) const
  {
    //  FileDriver driver;
    //  DBUsuario agregar;
    //  strcpy(agregar.Nombre, nombre.c_str());
    //  return driver.addUsuario(agregar);

    LOG_INFO("Contador: %i", m_ctx.GetContador());
    LOG_INFO("Usuarios en la DB: %i", m_ctx.Usuarios.GetRecordCount());

    //  imagino que usa move-semantics
    //
    // Usuario nuevo {
    //   .Email = id,
    //   .Password = pass,
    //   .Nombre = nombre,
    //   .Rol = rol
    // };


    Usuario nuevo {id.c_str(), nombre.c_str(), pass.c_str(), rol.c_str() };

    //  db.push_front(nuevo);
    m_ctx.Usuarios.AddEntity(nuevo);

    //  si tenemos un ctor en Usuario no podemos usar designated initializars
    //
    // m_ctx.Usuarios.AddEntity({
    //   .Email = id,
    //   .Password = pass,
    //   .Nombre = nombre,
    //   .Rol = rol
    // });

    LOG_INFO("Usuarios en la DB: %i", m_ctx.Usuarios.GetRecordCount());

    return true;
  }

}


