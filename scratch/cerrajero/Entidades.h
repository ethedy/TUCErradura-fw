/*

*/

#ifndef ENTIDADES_H
#define ENTIDADES_H

#include <string>
#include <cstdint>

namespace Entidades
{
  struct Usuario
  {
    std::string Email;
    std::string Nombre;
    std::string Password;
    std::string Rol;

    Usuario() = default;

    Usuario(const char* email, const char* nombre, const char* pass, const char* rol): Email(email), Nombre(nombre),
      Password(pass), Rol(rol)
    {
    }

    Usuario(const std::string& email, const std::string& nombre, const std::string& pass,
            const std::string& rol) : Email(email), Nombre(nombre), Password(pass), Rol(rol)
    {
    }
  };

  enum DiaSemana : uint8_t
  {
    Domingo = 0,
    Lunes,
    Martes,
    Miercoles,
    Jueves,
    Viernes,
    Sabado
  };

  struct Horario
  {
    DiaSemana Dia; //  usamos enum para mas placer
    uint8_t HoraInicio;
    uint8_t MinutoInicio;
    uint8_t HoraFin;
    uint8_t MinutoFin;
  };
};

#endif //ENTIDADES_H

