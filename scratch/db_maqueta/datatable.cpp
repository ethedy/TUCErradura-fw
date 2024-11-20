
#include "datatable.h"
#include "database.h"


DataTable::DataTable(char* nombre)
{
  this->name = new char[strlen(nombre)+1];

  Database::get();
}

void DataTable::read_configuration()
{
}

void DataTable::save_configuration()
{

}




