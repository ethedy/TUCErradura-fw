/**
 *
 *
 * */

#ifndef DB_MAQUETA_USER_H
#define DB_MAQUETA_USER_H

#include "cstdio"
#include "cstring"

class User
{
private:
  int id;
  char* nombre;

public:
  User() = default;

  explicit User(char* nombre);

  ~User();
};

#endif //DB_MAQUETA_USER_H
