/**
*  Estructuras con los formatos fisicos de los datos que guardaremos en
 *  el medio persistente
 *
 *
 */

#ifndef DB_TYPES_H
#define DB_TYPES_H

#include "Config.h"

namespace Database
{
  ///
  ///
  ///
  struct DBUsuario
  {
    char Id[MAX_EMAIL_LENGTH+1];
    char Nombre[MAX_NOMBRE_LENGTH+1];
    char Rol ;                            //  U user - A admin
    char Pass[MAX_PASSWORD_LENGTH];       //  hashed
  } ;


}


#endif //DB_TYPES_H
