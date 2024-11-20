
#include "driver_file.h"
#include <stdio.h>
#include <cstring>

DBUsuario 
FileDriver::findUsuario(std::string id, bool& found)
{
  //  llevarlo al ctor
  //
  FILE* dbUsers =  fopen("userdb", "r");

  //  por defecto no lo encontramos
  found = false;

  //  validar que el archivo exista...

  DBUsuario* pattern = new DBUsuario();
  
  while (fread(pattern, sizeof(DBUsuario), 1, dbUsers) == 1)
  {
    if (strcmp(pattern->email, id.c_str()) == 0)
    {
      //  encontrado, retornar este dato

      delete pattern;
      found = true;
      return (*pattern);  //  obvio que no
    }
  }
  delete pattern;

  fclose(dbUsers);  //  al dtor
  return *pattern;  //  obvio que no
}


bool 
FileDriver::addUsuario(DBUsuario nuevo)
{
  //  llevarlo al ctor
  //
  FILE* dbUsers =  fopen("userdb", "w");

  //  validar que el archivo exista...
  //  ir al final del archivo

  DBUsuario* pattern = new DBUsuario();
  
  while (fread(pattern, sizeof(DBUsuario), 1, dbUsers) == 1)
  {
    if (strcmp(pattern->email, id.c_str()) == 0)
    {
      //  encontrado, retornar este dato

      delete pattern;
      found = true;
      return (*pattern);  //  obvio que no
    }
  }
  delete pattern;

  fclose(dbUsers);  //  al dtor
  return *pattern;  //  obvio que no

}