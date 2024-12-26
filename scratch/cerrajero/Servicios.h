/*
 *
 *
 *
 */

#ifndef SERVICIOS_H
#define SERVICIOS_H

//  lamentablemente tengo que usar esta libreria de cuarta y no std::string
//  #include <WString.h>
//  #include "c_types.h"

#include <cstdint>
#include <optional>
#include <vector>
#include <string>
#include <regex>

#include <WString.h>
#include "Config.h"
#include "Repositorios.h"


namespace Servicios
{
  using namespace Datos;

  //using String = WString ;

  // TODO cambiar id por email
  // nombre de usuario es user
  //
  /*
    struct para guardar la info de un un usuario que tiene una sesion establecida en la cerradura

    Renombramos 'Session' a 'UserSession' para evitar el conflicto
  */
  struct UserSession
  {
    String Email;     //  correo (ID)
    String User;      //  nombre del usuario
    //  String PassHash;          //  Usaremos el hash de la contraseña
    String Rol;               //  "admin" o "user"
    String Token;             //  Token de sesión único para cada usuario
    uint64_t LastLogin;       //  valor de milisegundos que representa el tiempo de login, NO ES fecha/hora
    //
    String Status;          //  texto del error, o bien OK
    uint16_t StatusCode;    //  0 => no error, de otra manera el codigo del error
    //
    //  tal vez tengamos que agregar ctor y otros metodos
  };

  // struct Horario {
  //   DiaSemana Dia;              //  usamos enum para mas placer
  //   uint8_t HoraInicio;
  //   uint8_t MinutoInicio;
  //   uint8_t HoraFin;
  //   uint8_t MinutoFin;
  // };


  class ServiciosDatabase
  {
    friend class ServiciosSeguridad;

  private:
    RepositorioUsuarios m_userRepository;

  public:
    bool AddUser(const String& user_email, const String& user_name, const String& user_pass, const String& user_rol);

    std::optional<Usuario> GetUserByEmail(const String& user_email);

    //  update usuario???

    //  update horario/horarios???

    bool AddHorario(const String& user_email);

    bool AddHorario(const String& user_email, std::vector<Horario>& horarios);

    std::vector<Usuario> GetUsuarios();

    std::vector<Horario> GetHorariosFromUser(const String& user_email);

  private:
    std::optional<Usuario> GetUserByEmail(const std::string& user_email);
  };


  class ServiciosSeguridad
  {
  public:
    UserSession LoginUser(const String& user_email, const String& user_pass);

    bool UserAuthorized(const String& user_email, const String& datetime);

    //  void LogUserAccess();
  };


  class ServiciosValidacion
  {
  private:
    //  cambiar por un diccionario String, list<String>
    //
    std::forward_list<std::string> m_errores;
    std::regex m_emailNameValidator{R"(^([a-zA-Z0-9!#$%&'*+/=?^_`{|}~-]+(\.[a-zA-Z0-9!#$%&'*+/=?^_`{|}~-]+)*)$)"};
    std::regex m_emailDomainValidator{R"(^[a-zA-Z0-9]+(-[a-zA-Z0-9]+)*(?:\.[a-zA-Z]{2,})+$)"};

  public:
    bool ValidarEmail(const std::string& email);

    bool ValidarEmailNombre(const std::string& nombre);

    bool ValidarEmailDominio(const std::string& dominio);

    bool ValidarNombre(const std::string& nombre);

    bool ValidarPassword(const std::string& pass);

    bool ValidarRol(const std::string& rol);

    //  podriamos hacer un parse? o un ctor?
    //  un dia por vez...
    //  es mas probable que la habilitacion sea Lun y Mie por ejemplo y no los 5 dias
    //
    Horario BuildHorario(const String& dia, const String& hora_inicio, const String& hora_fin);

    bool ValidarFecha();

    bool ValidarHora();

  private:
    bool IsNullOrWhitespace(const std::string& value);
  };

};


#endif //SERVICIOS_H
