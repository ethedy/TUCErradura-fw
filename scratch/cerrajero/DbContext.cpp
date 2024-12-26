//
//
//

#include "DbContext.h"

#include "driver_memory.h"

namespace Datos
{
  DbContext::DbContext(Driver& backend) : m_contador{},
                           Usuarios(backend)
  {
    //  setear los drivers correspondientes
    //
    //Usuarios = new DatabaseSet<DBUsuario>();
  }

  DbContext& DbContext::GetInstance()
  {
    static MemoryDriver driver {};

    static DbContext instance(driver);

    return instance;
  }

  int
  DbContext::GetContador()
  {
    return m_contador++;
  }

}
