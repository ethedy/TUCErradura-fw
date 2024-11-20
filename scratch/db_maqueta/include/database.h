/**
 *
 *
 *
 */

#ifndef DB_MAQUETA_DATABASE_H
#define DB_MAQUETA_DATABASE_H

#include "users.h"
#include "cstdio"
#include <functional>
#include <datatable.h>

typedef std::function<void(const char* tag, const char* mensaje)> log_callback;

/**
 *  @brief proporciona servicios basicos necesarios para una base de datos como por ejemplo
 *  - guardar/recuperar configuraciones
 *  - mantener una lista de las tablas
 *  - disponibilizar un query engine que retorne informacion util a otras capas
 *
 *  Si lo vemos desde el mundo NET esto seria lo mas parecido a un DbContext...
 *  Entonces el repositorio deberia instanciar una database (unica) recuperar los datos si fuera necesario (hidratar)
 *  y luego prepararse para las consultas, no vamos a tener DbSet<T> por ahora pero podemos pensar en algo asi...
 *
 * */
class Database
{
private:
  Users* usuarios;

  FILE* config_file;
  char* base_path;
  char* config_file_name;
  log_callback db_callback;

  std::vector<DataTable> tables;

  //  usos multiples...
  //
  char buffer[150];

  Database();

  void log_to(const char* tag, const char* msg);

public:
  /*
   *  [[SINGLETON]]
   *  Database deberia ser unica
   *
   * */
  static Database& get();

  /**
   *  Completamos el singleton segun https://stackoverflow.com/questions/1008019/how-do-you-implement-the-singleton-design-pattern
   *  para que el ctor de copia y el operador de asignacion no generen copias indeseadas del singleton
   * */
  Database(Database const& db) = delete;

  void operator=(Database const& db) = delete;

  /**
   *  Establecemos una configuracion inicial para la base de datos donde incluimos el path base de todos los archivos y el nombre
   *  del archivo de configuracion de la DB
   *
   * */
  void use_configuration(char* base_path, char* cfg_file_name);

  FILE* get_config_file();

  /**
   *  Agrega una nueva tabla de datos a la base, permitiendo que pueda ser accedida desde una conexion
   */
  void add_datatable(DataTable& table);

  /**
   *  Obtiene una tabla
   */
  DataTable& get_datatable(const char* db_type);

  void set_logging_callback(log_callback cbk);
};

#endif //DB_MAQUETA_DATABASE_H
