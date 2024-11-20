//
//
//

#include "user.h"
#include "cstdio"
#include "cstring"
#include "database.h"

User::User(char* nombre)
{
  this->id = 0;
  this->nombre = new char[strlen(nombre)];
  strcpy(this->nombre, nombre);
}

User::~User()
{
  delete[] nombre;
}
