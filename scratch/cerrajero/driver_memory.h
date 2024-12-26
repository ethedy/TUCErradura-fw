//
//
//

#ifndef DRIVER_MEMORY_H
#define DRIVER_MEMORY_H

#include <string>
#include <forward_list>

#include "db_types.h"
#include "driver.h"

namespace Database
{
  using namespace Entidades;

  class MemoryDriver : public Driver
  {
  private:
    std::forward_list<DBUsuario> m_usuarios;    //  siempre guardamos el tipo mas basico

  public:
    MemoryDriver();

    bool AddUser(const Usuario& user) override;

    bool AddUser(const Usuario&& user) override;

    std::optional<Usuario> GetFromId(const std::string& id) override;

    int GetRecordCount(const Datos::Dataset& origen) override;
  };

}


#endif //DRIVER_MEMORY_H
