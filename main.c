#include "config.h"
#include "proxy.h"
#include "utils.h"
#include <stdio.h>


int main() {
  // 1. Load Configuration
  ProxyConfig config;
  load_config(&config);

  // 2. Start Proxy
  // This will enter the main loop
  start_proxy_server(&config);

  return 0;
}
