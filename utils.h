#ifndef UTILS_H
#define UTILS_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#endif

// Log levels
typedef enum { LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_DEBUG } LogLevel;

// Function prototypes
void log_message(LogLevel level, const char *format, ...);
void fail_with_message(const char *message);
char *get_current_time_string();

#endif // UTILS_H
