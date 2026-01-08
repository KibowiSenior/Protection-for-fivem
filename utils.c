#include "utils.h"

// Get current time as string
char* get_current_time_string() {
    static char buffer[20];
    time_t timer;
    struct tm* tm_info;

    timer = time(NULL);
    tm_info = localtime(&timer);

    strftime(buffer, 20, "%Y-%m-%d %H:%M:%S", tm_info);
    return buffer;
}

// Log message to console
void log_message(LogLevel level, const char *format, ...) {
    va_list args;
    const char *level_str;

    switch (level) {
        case LOG_INFO:    level_str = "[INFO]"; break;
        case LOG_WARNING: level_str = "[WARN]"; break;
        case LOG_ERROR:   level_str = "[ERROR]"; break;
        case LOG_DEBUG:   level_str = "[DEBUG]"; break;
        default:          level_str = "[UNKNOWN]"; break;
    }

    printf("%s %s ", get_current_time_string(), level_str);

    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("\n");
}

// Exit program with error message
void fail_with_message(const char *message) {
    log_message(LOG_ERROR, message);
    log_message(LOG_ERROR, "Error Code: %d", WSAGetLastError());
    exit(EXIT_FAILURE);
}
