//
// Created by ethed on 17/10/2024.
//

#ifndef DB_MAQUETA_DATATABLE_H
#define DB_MAQUETA_DATATABLE_H

class DataTable
{
private:
  char* name;

public:
  explicit DataTable(char* nombre); //  marcado explicit para evitar conversiones indeseadas

  void read_configuration();

  void save_configuration();

  const char* get_dbtype() { return name; }


};

#endif //DB_MAQUETA_DATATABLE_H
