//
//
//

#include <typeinfo>
#include <cstring>
#include <algorithm>

#include "driver_memory.h"
#include "DatasetUsuarios.h"
#include "Utiles.h"

namespace Database
{
  using namespace std;
  using namespace Datos;


  MemoryDriver::MemoryDriver()
  {
    LOG_WARNING("Llamado ctor de MemoryDriver");
  }

  // //  si no hay elementos en la lista, retornar false
  // if (!db.empty())
  // {
  //   //  buscar el elemento y retornar una copia/referencia??
  //   auto iter = find_if(db.begin(), db.end(), [&](UsuarioDTO u) { return u.Email == id;});
  //
  //   if (iter != db.end())
  //   {
  //     result.first = *iter;
  //     result.second = true;
  //   }
  // }

  bool MemoryDriver::AddUser(const Usuario& nuevo)
  {
    //  hacemos un casting a DBUsuario
    DBUsuario addUser;

    LOG_DEBUG("Insertando nuevo usuario...COPY...");

    strcpy(addUser.Id, nuevo.Email.c_str());
    strcpy(addUser.Nombre, nuevo.Nombre.c_str());
    strcpy(addUser.Pass, nuevo.Password.c_str());

    addUser.Rol = (nuevo.Rol == "admin") ? 'A' : 'U';

    m_usuarios.push_front(addUser);

    return true;
  }

  bool MemoryDriver::AddUser(const Usuario&& nuevo)
  {
    //  LOG_WARNING("Insertando nuevo usuario (move semantics)...");

    LOG_ERROR("Operacion no permitida: move operation en AddUser");

    //m_usuarios.push_front(nuevo);
    //  throw "Operacion no permitida: move operation en AddUser";

    return true;
  }

  std::optional<Usuario>
  MemoryDriver::GetFromId(const std::string& id)
  {
    auto iterUser = find_if(m_usuarios.begin(), m_usuarios.end(),
                            [&id](DBUsuario u) {
                              if (strcmp(u.Id, id.c_str()) == 0)
                              {
                                LOG_DEBUG("Usuario %s encontrado en la DB fisica", u.Id);
                                return true;
                              }
                              return false;
                            });

    if (iterUser == m_usuarios.end())
      return {};

    Usuario user { iterUser->Id, iterUser->Nombre, iterUser->Pass, iterUser->Rol == 'A' ? "admin" : "user" };

    return user;
  }


  int MemoryDriver::GetRecordCount(const Dataset& origen)
  {
    LOG_WARNING("Calculando record count en driver de memoria");

    // if (typeid(origen) == typeid(DatasetUsuarios))
    //   return std::distance(m_usuarios.begin(), m_usuarios.end());

    // throw exception();
    int userCuenta = std::distance(m_usuarios.begin(), m_usuarios.end());

    return userCuenta;
  }
}
