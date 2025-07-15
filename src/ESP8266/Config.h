
#ifndef CONFIG_H
#define CONFIG_H

#include <chrono>

using namespace std::chrono;
using namespace std::chrono_literals;

//  Limites para los diferentes datos almacenados en la DB

#define MIN_EMAIL_LENGTH          10
#define MAX_EMAIL_LENGTH          400
#define MAX_EMAIL_NAME_LENGTH     18

#define MIN_NOMBRE_LENGTH          6
#define MAX_NOMBRE_LENGTH         30

#define MAX_PASSWORD_LENGTH       20

//  Configuracion del logging por serial/stdout

#define LOG_INCLUDE_TIMESTAMP   
#define LOG_INCLUDE_LOG_LEVEL

#define EEPROM_BEGIN_ADDRESS  0

/*
  Constantes de temporizacion varias
*/

// Es el tiempo que permanece valido un token generado para un usuario normal (no admin)
//
const minutes User_Token_Expiration_Time = 3min;

/// Es el tiempo que permanece valido un token generado para un usuario privilegiado
//
const minutes Admin_Token_Expiration_Time = 3min;

//const uint32_t Watchdog_Time = 10 * 1'000'000;          //  pasarlo a segundos
const seconds Watchdog_Time = 45s;

//const uint32_t ClearSession_Time = 25 * 1'000'000UL;    //  pasarlo a segundos
const seconds Purge_Tokens_Time = 5min;

#endif //   CONFIG_H
