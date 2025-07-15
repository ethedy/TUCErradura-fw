/*
 *
 *
 *
 */


#ifndef REPOSITORIOS_H
#define REPOSITORIOS_H

#include <string>
#include <optional>

#include "DbContext.h"
#include "DriverMemory.h"

namespace Datos
{
  class RepositorioUsuarios
  {
  private:
    DbContext& m_ctx;

  public:
    RepositorioUsuarios(): m_ctx(DbContext::GetInstance()) { }

    std::optional<Usuario> GetUserFromId(const std::string& id) const;

    bool AddUser(const std::string& id, const std::string& nombre, const std::string& pass, const Entidades::Roles rol) const;

    bool IsEmpty() const;
  };

}


#endif //REPOSITORIOS_H
