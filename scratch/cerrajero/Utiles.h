/*
 *  Mejoras y ayudas globales para todo tipo de uso
 *
 *
 *
 */

#ifndef UTILES_H
#define UTILES_H

#include <cstdint>
#include <string>

#define LOG_INFO(msg, ...) Log(__func__, LogLevel::Information, msg, ##__VA_ARGS__)
#define LOG_WARNING(msg, ...) Log(__func__, LogLevel::Warning, msg, ##__VA_ARGS__)
#define LOG_DEBUG(msg, ...) Log(__func__, LogLevel::Debug, msg, ##__VA_ARGS__)
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

inline LogLevel g_min_log_level = LogLevel::None;

void DumpMemory(const void* ptr, size_t size, const char* msg = nullptr);

/// Recorta una cadena de espacios en blanco al inicio y al final
/// Consideramos espacios en blanco lo mismo que normalmente se usa en expresiones regulares:
/// - space
/// - CR
/// - LF
/// - TAB vertical
/// - TAB horizontal
///
std::string AllTrim(const std::string& source);

void Log(const char* from, LogLevel log_level, const char* frmt, ...);

void LogTest(const char* from);

#endif //UTILES_H
