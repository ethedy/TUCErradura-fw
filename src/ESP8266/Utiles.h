#include "core_esp8266_features.h"

/*
  Mejoras y ayudas globales para todo tipo de uso
 
 
 
*/

#ifndef UTILES_H
#define UTILES_H

#include <EEPROM.h>
#include <Schedule.h>

#include <cstdint>
#include <string>
#include <functional>
#include <chrono>
#include <cstring>

#include "Entidades.h"

using namespace std::chrono_literals;

#define LOG_VERBOSE(msg, ...) Log(__func__, LogLevel::Verbose, msg, ##__VA_ARGS__)
#define LOG_DEBUG(msg, ...) Log(__func__, LogLevel::Debug, msg, ##__VA_ARGS__)
#define LOG_INFO(msg, ...) Log(__func__, LogLevel::Information, msg, ##__VA_ARGS__)
#define LOG_WARNING(msg, ...) Log(__func__, LogLevel::Warning, msg, ##__VA_ARGS__)
#define LOG_ERROR(msg, ...) Log(__func__, LogLevel::Critical, msg, ##__VA_ARGS__)

enum LogLevel : uint8_t
{
  Verbose,
  Debug,
  Information,
  Warning,
  Critical,
  None = 255
};

/*
      ---------------------------------------------------
                          GLOBALES
      ---------------------------------------------------
*/

inline LogLevel g_min_log_level = LogLevel::None;

/* 
  Indica si la fecha/hora que esta mostrando el ESP8266 mediante la cuenta del RTC esta recientemente ajustada
  desde algun header de la aplicacion o desde algun otro medio externo
*/
inline bool g_tiempo_ajustado = false;

/*
      ---------------------------------------------------
                          FUNCIONES
      ---------------------------------------------------
*/

/*
  Sencilla funcion para mostrar en consola el contenido de memoria del micro
  @params ptr poppop
*/
void DumpMemory(const void* ptr, size_t size, const char* msg = nullptr);

/*
  Recorta una cadena de espacios en blanco al inicio y al final
  Consideramos espacios en blanco lo mismo que normalmente se usa en expresiones regulares:
  - space
  - CR
  - LF
  - TAB vertical
  - TAB horizontal
*/
std::string AllTrim(const std::string& source);

/*
  Retorna true si el contenido de la cadena esta vacio o bien si solo contiene caracteres no imprimibles (CR, tabulaciones...)
  Recordar que en C++ una variable o parametro no puede ser null a no ser que se trate de un puntero
  por esa razon no es como en C# IsNullOrWhitespace
*/ 
bool IsEmptyOrWhitespace(const std::string& value);

/* 
  Por ahora solo convierte cadenas en hexadecimal
  Es un helper para validar el token ademas nos sirve para que no venga cualquier cosa en dicho token
*/
uint64_t stoull_seguro(const std::string& s);

Entidades::Roles ParseRol(const std::string& rol);

std::string ToString(const Entidades::Roles rol);

/* 
  Retorna la fecha/hora almacenada en el uC como una cadena null terminated con el formato:
  YYYY-MM-DD hh:mm:ss

  TODO: podriamos mostrarla diferente si no es exacta?
*/
const char* GetCurrentDatetime();

/*
  Ajusta la fecha/hora en el uC con el valor que le pasamos
  Ademas tiene un flag para indicar si la fecha/hora es exacta o no, de ser exacta la persistimos en EEPROM y ademas
  ese flag nos sirve para evitar estar cambiando la hora con cada header que contenga dicho dato
*/
void SetDateTime(time_t dt, bool exacta = false);

/*
  Parsea una fecha en el formato EXACTO YYYY-MM-DD HH:mm:ss

  Validamos que existan los separadores y limites superiores de mes, dia y tiempos
  Si no encontramos el separador vamos a retornar el no-valor

  Usa mktime() que es extremadamente permisiva
*/
std::optional<time_t> ParseDateTime(const std::string& input);

void Log(const char* from, LogLevel log_level, const char* frmt, ...);

void LogTest(const char* from);

/*
      ---------------------------------------------------
                    TEMPLATE FUNCTIONS
      ---------------------------------------------------
*/

/*
  Mejora de la funcion de sistema schedule_recurrent_function_us para aceptar como argumento una duration<T> y ademas
  cualquier valor (horas...dias...)
*/ 
template<typename Rep, typename Period>
void ScheduleFuntion(const std::function<bool(void)>& fn, const std::chrono::duration<Rep, Period> lapso)
{
  //  esto es para saber por ejemplo el numerador o denominador del periodo usado para la duracion actual
  //  using Ratio = typename std::chrono::duration<Rep, Period>::period;

  std::chrono::seconds segundos = std::chrono::duration_cast<std::chrono::seconds>(lapso);

  if (segundos <= 25s)
  {
    //  printf("Usando API directa...\n");

    schedule_recurrent_function_us(fn, std::chrono::duration_cast<std::chrono::microseconds>(segundos).count());
  }
  else
  {
    //  la cantidad de veces que habra que llamar a la funcion intermediaria hasta que se llame a la principal
    //
    int repeticiones = segundos.count();

    //  Se usa un parametro cuenta para que en cada llamada a la misma instancia del template, este parametro sea diferente
    //  y por lo tanto las cuentas no se compartan
    //  Tiene que declararse mutable para que las capturas puedan ser modificadas dentro de la fn lambda
    //
    schedule_recurrent_function_us([repeticiones, fn, cuenta=0]() mutable {
      //  no podemos usar una variable static porque si bien funciona con diferentes instancias del template, cuando los type-parameters
      //  son los mismos se va a compartir la misma variable y no va a funcionar como uno quiere, o sea puede ser que siempre se ejecute
      //  una de las funciones o se ejecuten alternadas...pero por supuesto no es el comportamiento deseado
      //  
      //  static int cuenta = 0;

      //  LOG_VERBOSE
      //  printf("ScheduleFunction...\n");

      if (++cuenta >= repeticiones)
      {
        cuenta = 0;
        //  LOG_VERBOSE
        //  printf("Llamando a fn() ScheduleFunction y reseteando contador...\n");
        return fn();
      }

      return true;
    }, std::chrono::duration_cast<std::chrono::microseconds>(1s).count());
  }
}

/*
      ---------------------------------------------------
                          CLASES
      ---------------------------------------------------
*/

/*
  Sencilla clase que permite realizar el seguimiento de los tiempos que demoran determinadas operaciones
*/
class StopWatch
{
private:
  std::chrono::microseconds start;
  using fpMilisegundos = std::chrono::duration<float, std::chrono::milliseconds::period>;

public:
  void Start() 
  {
    start = std::chrono::microseconds(micros());
  }

  std::chrono::microseconds Elapse()
  {
    return std::chrono::microseconds(micros()) - start;
  }

  std::chrono::duration<float, std::chrono::milliseconds::period>
  ElapseFloat()
  {
    return fpMilisegundos(Elapse());
  }
};

struct DatabaseConfig
{
  int32_t LastId;
  int32_t RowCount;
  bool IsFragmentated;
};

/*
  Interface con EEPROM en 8266 para guardar variables que deberian persistir entre reinicios o reflasheos
*/
struct PersistentConfig
{
private:  
  char Mark[10];

public:  
  std::time_t LastTimeSaved;
  DatabaseConfig ConfigUsuarios;
  DatabaseConfig ConfigHorarios;

public:
  PersistentConfig() 
  {
    EEPROM.begin(512);
  }

  ~PersistentConfig() 
  {
    EEPROM.end();
  }

  void Read() 
  {
    EEPROM.get<PersistentConfig>(0, *this);
  }

  /*
    Inicialmente o sea cuando todavia no hemos guardado nada en la EEPROM la marka esta vacia por lo tanto HasData()
    retornara false
  */
  bool HasData()
  {
    return (strcmp(Mark, "TUSERULES") == 0);
  }

  void Save()
  {
    StopWatch sw;
    sw.Start();

    strcpy(Mark, "TUSERULES");
    EEPROM.put<PersistentConfig>(0, *this);
    EEPROM.commit();
    
    LOG_INFO("EEPROM commit time: %7.3fms", sw.ElapseFloat().count());
  }

  /*
    Este metodo deja todas las configuraciones en cero, similar a lo que se tendria cuando iniciamos
    el firmware por primera vez
  */
  void Erase()
  {
    std::memset(Mark, 0, sizeof Mark);
    LastTimeSaved = -1;
    ConfigUsuarios = {};
    ConfigHorarios = {};

    EEPROM.put<PersistentConfig>(0, *this);
    EEPROM.commit();
  }
};

#endif //UTILES_H
