//
// Created by ethed on 14/12/2024.
//

#ifndef DRIVER_FILE_H
#define DRIVER_FILE_H

#include <string>
#include "db_types.h"
#include "Entidades.h"


//  posiblemente tengamos que poner un header con la cantidad de registros existentes
//  ojo si borramos un elemento en el medio...
//  capaz que hay que defragmentar
//
class FileDriver
{
private:
  //const
  int record_count;

public:
  FileDriver()
  {
    open_file_and_setup();
  }

  Entidades::Usuario FindUsuario(std::string& id);

  bool AddUsuario(Entidades::Usuario& nuevo);

private:
  void open_file_and_setup();

  void get_record_count() noexcept;
};

#endif //DRIVER_FILE_H
