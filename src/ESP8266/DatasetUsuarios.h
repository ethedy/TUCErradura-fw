
#ifndef DATASETUSUARIOS_H
#define DATASETUSUARIOS_H

#include <forward_list>
#include <string>

#include "Dataset.h"
#include "Entidades.h"
#include "Driver.h"


namespace Datos
{
  using namespace Entidades;
  using namespace Database;

  ///   Mi idea aca era replicar el DbSet<T> de EF pero claramente no va a ser tan sencillo
  ///
  class DatasetUsuarios : Dataset
  {
  private:
    Driver& m_driver;
    std::forward_list<Usuario> m_cache;

  public:
    DatasetUsuarios(Driver& backend);

    void AddEntity(const Usuario& item);

    void AddEntity(const Usuario&& item);

    std::optional<Usuario> Find(const std::string& id);

    int GetRecordCount() override;
  };

};


#endif //DATASETUSUARIOS_H
