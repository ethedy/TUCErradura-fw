/*

*/

#ifndef ENTIDADES_H
#define ENTIDADES_H

#include <ArduinoJson.h>

#include <string>
#include <cstdint>

namespace Entidades
{
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

  enum Roles : uint8_t
  {
    User_Rol,
    Admin_Rol,
    Unknown_Rol = 200
  };

  enum AuthorizationErrors : uint8_t
  {
    Token_Not_Found,
    Token_Expired,
    Token_Invalid,
    No_Autorizado,
    No_Autenticado
  };

  struct Usuario
  {
    int32_t Id;
    std::string Email;
    std::string Nombre;
    std::string Password;
    //std::string Rol;
    Entidades::Roles Rol;

    Usuario(): Id(-1) {};

    Usuario(const char* email, const char* nombre, const char* pass, const Entidades::Roles rol): Email(email), Nombre(nombre),
      Password(pass), Rol(rol), Id(-1)
    {
    }

    Usuario(const std::string& email, const std::string& nombre, const std::string& pass,
            const Entidades::Roles rol) : Email(email), Nombre(nombre), Password(pass), Rol(rol), Id(-1)
    {
    }
  };

  struct Horario
  {
    int32_t Id;
    DiaSemana Dia; //  usamos enum para mas placer
    uint8_t HoraInicio;
    uint8_t MinutoInicio;
    uint8_t HoraFin;
    uint8_t MinutoFin;
    //
    int32_t IdUsuario;
  };

  struct ProblemDetailResult
  {
    int Status;
    std::string Title;

    std::string ToJson()
    {
      JsonDocument json;
      std::string result {};

      json["status"] = Status;
      json["title"] = Title;

      serializeJson(json, result);

      return result;
    }
  };

  struct LoginResult
  {
    int Status;
    std::string Mensaje;
    std::optional<uint64_t> Token;
    
    //  JsonDocument json;

    LoginResult(int status, const std::string& mensaje)
    {
      Token = {};
      Status = status;
      Mensaje = mensaje;
    }

    std::string ToJson() 
    {
      JsonDocument json;
      std::string result {};
      char buffer[30];

      json["status"] = Status;
      json["mensaje"] = Mensaje;
      
      if (Token)
      {
        snprintf(buffer, 30, "%016llx", *Token);
        json["token"] = buffer;
      }
      
      serializeJson(json, result);

      return result;
    }
  };

  struct InfoResult
  {
    //  mac address...
    std::string MacAddress;

    std::string ToJson()
    {
      JsonDocument json;
      std::string result {};

      json["mac_address"] = MacAddress;

      serializeJson(json, result);
      return result;
    }
  };

};

#endif //ENTIDADES_H

