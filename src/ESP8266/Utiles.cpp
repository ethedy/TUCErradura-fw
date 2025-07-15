/*

*/

#include <cstdarg>
#include <ctime>
#include <regex>
#include <cctype>
#include <string>

#include <sys/time.h>

#include "Utiles.h"
#include "Config.h"

#ifdef ARDUINO_ESP8266_WEMOS_D1R1
  #include <HardwareSerial.h>

  extern HardwareSerial Serial;
  
#endif

using namespace std::string_literals;

extern PersistentConfig Configuration;

const char* GetCurrentDatetime()
{
  static char buffer[50];  //  para mostrar fecha/hora simples

  std::time_t t_ticks = time(nullptr);
  std::tm* t_local = localtime(&t_ticks);  //  ojo este struct esta creado en el sistema

  std::strftime(buffer, 49, "%F %X", t_local);

  return buffer;
}

/*
*/
void SetDateTime(time_t dt, bool exacta)
{
  struct timeval timeValue {
    .tv_sec = dt,
    .tv_usec = 0
  };
  
  settimeofday(&timeValue, nullptr);

  if (exacta)
  {
    g_tiempo_ajustado = true;

    //  guardamos en EEPROM
    //
    Configuration.LastTimeSaved = dt;
    Configuration.Save();
  }
}

std::optional<time_t> ParseDateTime(const std::string& input)
{
  std::tm tiempo {};

  if (input.size() != 19)
    return {};

  if (input[4] == '-' && input[7] == '-' && (input[10]==' ' || input[10]=='T') && input[13]==':' && input[16]==':')
  {
    //  todo OK!!
    tiempo.tm_year = std::stoi(input.substr(0, 4)) - 1900;    //  el año es siempre base 1900
    tiempo.tm_mon = std::stoi(input.substr(5, 2)) - 1;        //  el mes arranca desde CERO
    tiempo.tm_mday= std::stoi(input.substr(8, 2));
    tiempo.tm_hour= std::stoi(input.substr(11, 2));
    tiempo.tm_min= std::stoi(input.substr(14, 2));
    tiempo.tm_sec = std::stoi(input.substr(17, 2));

    //  tengo que validar porque mktime es muy permisiva...
    //
    if (tiempo.tm_mon > 11 || tiempo.tm_mday > 31 || tiempo.tm_hour > 23 || tiempo.tm_min > 59 || tiempo.tm_sec > 59)
      return {};

    std::time_t result = mktime(&tiempo);

    if (result == -1)
      return {};

    return result;
  }
  return {};
}

std::string AllTrim(const std::string& source)
{
  const char* whitespaces = " \n\r\t\f\v";

  size_t inicial = source.find_first_not_of(whitespaces);

  //  is no hay ningun caracter distinto a WS, find_first_not_of() retorna la constante string::npos que tiene un valor
  //  de -1 pero casteado a un size_type definido dentro del template de basic_string<>
  //
  if (inicial == std::string::npos)
    return {};

  size_t final2 = source.find_last_not_of(whitespaces);

  return source.substr(inicial, final2 - inicial + 1);
}

//
bool IsEmptyOrWhitespace(const std::string& value)
{
  std::regex rx_vacio{"[[:s:]]*"};

  return std::regex_match(value, rx_vacio);
}

//
uint64_t stoull_seguro(const std::string& s)
{
  uint64_t result = -1;

  if (std::all_of(s.begin(), s.end(), [](char x){ return std::isxdigit(x) != 0;}))
  {
    result = std::stoull(s, nullptr, 16);
  }
  return result;
}

//
Entidades::Roles ParseRol(const std::string& rol)
{
  std::regex ex_roles(R"(^(admin)$|^(user)$|^(guest)$)", std::regex::icase);
  std::smatch match;

  if (std::regex_match(rol, match, ex_roles))
  {
    if (match[1].matched)
      return Entidades::Roles::Admin_Rol;
    if (match[2].matched)
      return Entidades::Roles::User_Rol;
  }
  return Entidades::Roles::Unknown_Rol;
}

std::string ToString(const Entidades::Roles rol)
{
  if (rol == Entidades::Roles::Admin_Rol)
    return {"Admin"s};
  else
    return {"User"s};
}

//
void LogTest(const char* from)
{
  auto i = sizeof(from);
  printf("Tamaño: %zd\n", i);
  printf("Desde: %s\n", from);
}

void
DumpMemory(const void* ptr, size_t size, const char* msg)
{
  const char* pp = static_cast<const char*>(ptr);
  char buffer[50] ;

  if (msg == nullptr)
    printf("Dump MM a partir de %p\n", pp);
  else
    printf("%s %p", msg, pp);

  strncpy(buffer, pp, 49);
  buffer[49] = 0;

  for (int i =0; i < size; i++)
  {
    uint8_t byte =  *((uint8_t*)buffer+i);

    printf("0x%.2x '%c' ", byte, byte);
  }
  fputc('\n', stdout);
}

//  TODO se podran usar colores?
//  TODO elegir serial y probar en la salida UART1
//  TODO mostrar o no time...pero de donde sacariamos el time?? podriamos calcularlo aproximadamente con milis() desde la ultima
//  hora recibida?

//  TODO cambiar por variadic templates ProfC++ 960
//  Referencia ProfC++ 447
//
void Log(const char* from, LogLevel log_level, const char* frmt, ...)
{
  if (log_level >= g_min_log_level)
  {
    std::string formato {};

    va_list params;
    va_start(params, frmt);

    formato.append(GetCurrentDatetime());
    
    //#ifdef LOG_INCLUDE_LOG_LEVEL
      switch (log_level)
      {
        case LogLevel::Verbose:
          formato.append(" [VERBOSE] ");
          break;
          
        case LogLevel::Debug:
          formato.append(" [DEBUG] ");
          break;

        case LogLevel::Information:
          formato.append(" [INFORMATION] ");
          break;

        case LogLevel::Warning:
          formato.append(" [WARNING] ");
          break;

        case LogLevel::Critical: 
          formato.append(" [CRITICAL] ");
          break;
      }
    //#endif

    //DumpMemory(from, 16, "Desde LOG ");
    //Serial.printf(from);
    
    char buffer[100];
    buffer[99] = 0x00;
    strncpy(buffer, from, 99);
    
    formato.append("{");
    formato.append(buffer);
    formato.append("} ");

    //  Serial.printf(buffer);

    //  no funciona
    //  formato.append(from, strlen(from));
    
    //formato.append(" %i \n");
    
    formato.append(frmt);
    formato.append(1, '\n');

    //Serial.printf(formato.c_str());

    #ifdef ARDUINO_ESP8266_WEMOS_D1R1
      //Serial.printf("BOO");
      vfprintf(stdout, formato.c_str(), params);   //  usamos la version fprintf (con archivos) + va_list como argumento
    #else
      vfprintf(stdout, formato.c_str(), params);   //  usamos la version fprintf (con archivos) + va_list como argumento
    #endif
    
    va_end(params);
  }
}

