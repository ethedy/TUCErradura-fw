#include <iostream>

#include "database.h"
#include "users.h"
//  #include "user.h"

using namespace std;

/*
 *  Prueba de concepto para lo que seria una base de datos simple en el firmware del ESP8266
 *
 *  Los datos de guardan y recuperan de archivos binarios
 *  La configuracion de las tablas asociadas a las bases de datos se guardan tambien en archivos de configuracion adhoc
 *  Las tablas son vectores STL
 *  Las filas son instancias de clases C++
 *  Tratamos de usar librerias standard de C para menor footprint
 *
 * */


int main()
{
  cout << "Prueba de concepto de la base de datos para cerradura" << endl;

  Database& db_tuse = Database::get();

  db_tuse.set_logging_callback([](const char* tag, const char* msg){
    printf("<timestamp> %s %s", tag, msg);
  });

  db_tuse.use_configuration("db_base_path", "cfg_file_name");

  Users tbl_users;

  return 0;
}
