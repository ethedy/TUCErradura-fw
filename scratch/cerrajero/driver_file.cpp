//
// Created by ethed on 14/12/2024.
//

#include "driver_file.h"
#include <cstring>
#include <cstdio>

Entidades::Usuario
FileDriver::FindUsuario(std::string& id)
{
  //  llevarlo al ctor
  //
  FILE* dbUsers =  fopen("userdb", "r");

  //  por defecto no lo encontramos
  bool found = false;

  //  validar que el archivo exista...

  Entidades::Usuario* pattern = new Entidades::Usuario();

  while (fread(pattern, sizeof(Entidades::Usuario), 1, dbUsers) == 1)
  {
    if (strcmp(pattern->Email.c_str(), id.c_str()) == 0)
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
FileDriver::AddUsuario(Entidades::Usuario& nuevo)
{
  //  llevarlo al ctor
  //
  FILE* dbUsers =  fopen("userdb", "w");

  //  validar que el archivo exista...
  //  ir al final del archivo

  Entidades::Usuario* pattern = new Entidades::Usuario();

  while (fread(pattern, sizeof(Entidades::Usuario), 1, dbUsers) == 1)
  {
  }
  delete pattern;

  fclose(dbUsers);  //  al dtor
  return false;
}
