//
//
//

#include "database.h"
#include "users.h"
#include "cstdio"
#include <functional>


Database::Database()
{
  this->db_callback = nullptr;
}

Database& Database::get()
{
  static Database instance;

  return instance;
}

/*
 *  Ajusta los parametros de configuracion de la base de datos que podran usar las diferentes tablas
 *
 *
 * */
void Database::use_configuration(char* base_path, char* cfg_file_name)
{
  this->config_file_name = cfg_file_name;
  this->base_path = base_path;

  log_to("DB", "Creadas las variables de configuracion. Archivo abierto");
}

FILE* Database::get_config_file()
{
  //  TODO seccion critica...

  return config_file;
}

/**
 *  ver este warning!!
 *  Parameter 'cbk' is passed by value and only copied once; consider moving it to avoid unnecessary copies
 *  Tenia que ver con move semantics, uso std::move() por ahora para solucionarlo
 *  https://youtu.be/St0MNEU5b0o?si=AfYcEA1mTDYoasNp
 *
 * */
void Database::set_logging_callback(log_callback cbk)
{
  this->db_callback = std::move(cbk);
}

void Database::log_to(const char* tag, const char* msg)
{
  if (db_callback != nullptr)
    db_callback(tag, msg);
}

void Database::add_datatable(DataTable& table)
{
  tables.push_back(table);
}

DataTable& Database::get_datatable(const char* db_type)
{
  for (auto& dt: tables)
  {
    if (strcmp(dt.get_dbtype(), db_type) == 0)
      return dt;
  }
  throw 100;
}


