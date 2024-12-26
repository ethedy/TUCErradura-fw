//
//
//

#include "DatasetUsuarios.h"
#include "Utiles.h"

#include <algorithm>
#include <bits/error_constants.h>

namespace Datos
{
  using namespace Entidades;
  using namespace Database;

  DatasetUsuarios::DatasetUsuarios(Driver& backend) : m_driver(backend)
  {

  }

  void DatasetUsuarios::AddEntity(const Usuario& item)
  {
    LOG_DEBUG("Usando copy semantics");

    m_cache.remove_if([&item](Usuario u) {
      if (u.Email == item.Email)
      {
        LOG_DEBUG("Se elimina %s desde el cache", item.Email.c_str());
        return true;
      }
      return false;
    });

    m_driver.AddUser(item);
  }

  void DatasetUsuarios::AddEntity(const Usuario&& item)
  {
    LOG_DEBUG("Usando move semantics");

    m_cache.remove_if([&item](Usuario u) {
      if (u.Email == item.Email)
      {
        LOG_DEBUG("Se elimina %s desde el cache", item.Email.c_str());
        return true;
      }
      return false;
    });

    m_driver.AddUser(std::move(item));
  }

  std::optional<Usuario>
  DatasetUsuarios::Find(const std::string& id)
  {
    //  solo usamos cache cuando hacemos find, para que quede en la lista
    auto iterFound = std::find_if(m_cache.begin(), m_cache.end(), [&id](Usuario u) {
      if (u.Email == id)
      {
        LOG_DEBUG("Se encontro el item en el cache: %s", id.c_str());
        return true;
      }
      return false;
    });

    if (iterFound != m_cache.end())
      return *iterFound;

    auto result = m_driver.GetFromId(id);

    if (result)
      m_cache.push_front(result.value());

    return result;
  }

  int DatasetUsuarios::GetRecordCount()
  {
    return m_driver.GetRecordCount(*this);
  }

};
