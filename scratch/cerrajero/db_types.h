
/**
 *  Estructuras con los formatos fisicos de los datos que guardaremos en 
 *  el medio persistente
 * 
 * 
 */

#ifndef DB_TYPES_H
#define DB_TYPES_H

#include <stdio.h>

typedef struct 
{
  char nombre[100];
  char email[50];
  char pass[30];  //  hashed
  char rol;
} DBUsuario;



#endif
