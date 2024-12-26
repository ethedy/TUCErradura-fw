/*

*/

#include "Utiles.h"
#include "Config.h"

#include <cstdarg>

#ifdef ARDUINO_ESP8266_WEMOS_D1R1
  #include <HardwareSerial.h>

  extern HardwareSerial Serial;
  
#endif

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
    
    //#ifdef LOG_INCLUDE_LOG_LEVEL
      switch (log_level)
      {
        case LogLevel::Verbose:
          formato.append("[VERBOSE] ");
          break;
          
        case LogLevel::Debug:
          formato.append("[DEBUG] ");
          break;

        case LogLevel::Information:
          formato.append("[INFORMATION] ");
          break;

        case LogLevel::Warning:
          formato.append("[WARNING] ");
          break;

        case LogLevel::Critical: 
          formato.append("[CRITICAL] ");
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
