#include "config.h"
#include "utils.h"
#include <string.h>

void load_config(ProxyConfig *config) {
    // In a real scenario, we would parse a json/ini file here.
    // For now, setting defaults or reading from env could be done.
    
    // Default values
    config->listen_port = DEFAULT_LISTEN_PORT;
    strncpy(config->backend_ip, DEFAULT_BACKEND_IP, sizeof(config->backend_ip) - 1);
    config->backend_port = DEFAULT_BACKEND_PORT;
    
    // Protection defaults
    config->max_pps = 1000;      // Allow 1000 packets per second (high for gaming, adjust as needed)
    config->block_duration = 60; // Block for 1 minute
    
    log_message(LOG_INFO, "Configuration loaded:");
    log_message(LOG_INFO, "  Listen Port: %d", config->listen_port);
    log_message(LOG_INFO, "  Backend: %s:%d", config->backend_ip, config->backend_port);
    log_message(LOG_INFO, "  Protection: Max PPS=%d, Block Duration=%ds", config->max_pps, config->block_duration);
}
