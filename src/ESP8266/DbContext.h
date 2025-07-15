
//
//
//

#ifndef DBCONTEXT_H
#define DBCONTEXT_H

#include "DatasetUsuarios.h"

namespace Datos
{
  //  esta tiene que ser un singleton...
  //
  class DbContext
  {
  private:
    int m_contador;

  private:
    DbContext(Driver& backend);

  public:
    static DbContext& GetInstance();

    DbContext(const DbContext&) = delete;
    void operator=(const DbContext&) = delete;

    DatasetUsuarios Usuarios;

    //
    int GetContador();
  };

}


#endif //DBCONTEXT_H
