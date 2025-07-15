//
// Created by ethed on 17/12/2024.
//

#ifndef DRIVER_H
#define DRIVER_H

#include <optional>
#include <string>

#include "Dataset.h"
#include "Entidades.h"

namespace Database
{
  using namespace Entidades;

  //  clase abstracta/interface
  //
  class Driver
  {
  public:
    virtual ~Driver() = default;

    //  esto es un chino...
    //  TODO hay solucion a este problema???
    //
    virtual int GetRecordCount(const Datos::Dataset& origen) = 0;

    virtual bool AddUser(const Usuario& nuevo) = 0;

    virtual bool AddUser(const Usuario&& nuevo) = 0;

    //  siempre retornamos una referencia
    //  es responsabilidad del driver mantenerla en memoria
    //
    virtual std::optional<Usuario> GetFromId(const std::string& id) = 0;
  };

}


#endif //DRIVER_H
