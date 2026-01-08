#ifndef PROXY_H
#define PROXY_H

#include "config.h"

// Initialize and start the proxy server
// This function will enter the main loop and not return until error or signal
void start_proxy_server(const ProxyConfig *config);

#endif // PROXY_H
