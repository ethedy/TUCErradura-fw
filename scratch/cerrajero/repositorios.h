/*
 *
 *
 *
 */


#ifndef REPOSITORIOS_H
#define REPOSITORIOS_H

#include <string>

#include "DbContext.h"
#include "driver_memory.h"


namespace Datos
{
  class RepositorioUsuarios
  {
  private:
    DbContext& m_ctx;

  public:
    RepositorioUsuarios(): m_ctx(DbContext::GetInstance()) { }

    std::pair<Usuario, bool> GetUserFromId(const std::string& id) const;

    bool AddUser(const std::string& id, const std::string& nombre, const std::string& pass, const std::string& rol) const;
  };

}


#endif //REPOSITORIOS_H
