
#ifndef CONFIG_H
#define CONFIG_H

//  Limites para los diferentes datos almacenados en la DB

#define MIN_EMAIL_LENGTH          10
#define MAX_EMAIL_LENGTH          40
#define MAX_EMAIL_NAME_LENGTH     18

#define MIN_NOMBRE_LENGTH          6
#define MAX_NOMBRE_LENGTH         30

#define MAX_PASSWORD_LENGTH       20

//  Configuracion del logging por serial/stdout

#define LOG_INCLUDE_TIMESTAMP   
#define LOG_INCLUDE_LOG_LEVEL

#endif //   CONFIG_H
