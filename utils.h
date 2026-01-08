#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <winsock2.h>
#include <ws2tcpip.h>

// Log levels
typedef enum {
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_DEBUG
} LogLevel;

// Function prototypes
void log_message(LogLevel level, const char *format, ...);
void fail_with_message(const char *message);
char* get_current_time_string();

#endif // UTILS_H
