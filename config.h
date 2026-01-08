#ifndef CONFIG_H
#define CONFIG_H

#define DEFAULT_LISTEN_PORT 30120
#define DEFAULT_BACKEND_IP "127.0.0.1"
#define DEFAULT_BACKEND_PORT 30121

typedef struct {
    int listen_port;
    char backend_ip[64];
    int backend_port;
    int max_pps;           // Max packets per second per IP
    int block_duration;    // Duration in seconds to block abusive IPs
} ProxyConfig;

// Function prototypes
void load_config(ProxyConfig *config);

#endif // CONFIG_H
