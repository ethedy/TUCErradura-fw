//
//
//

#ifndef DB_MAQUETA_USERS_H
#define DB_MAQUETA_USERS_H

#include "user.h"
#include "datatable.h"
#include "vector"

class Users : DataTable
{
private:
  std::vector<User> users_table;   // el vector donde guardaremos en memoria las filas que representan a los usuarios

public:
  Users();
};

#endif //DB_MAQUETA_USERS_H
