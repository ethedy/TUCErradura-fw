#ifndef DRIVER_FILE_H
#define DRIVER_FILE_H

#include "db_types.h"
#include <stdio.h>
#include <string>

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

  DBUsuario findUsuario(std::string, bool& found);

  bool addUsuario(DBUsuario nuevo);

private:
  void open_file_and_setup();

  void get_record_count() noexcept;
};



#endif