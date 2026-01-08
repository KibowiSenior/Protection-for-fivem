#define FD_SETSIZE 1024 // Increase select limit
#include "proxy.h"
#include "utils.h"
#include <stdio.h>

// --- Session Structures ---
typedef struct {
  struct sockaddr_in client_addr;
  SOCKET backend_socket; // Socket used to talk to backend for this client
  time_t last_activity;
  int active;

  // Rate Limiting
  time_t pps_ts;
  int pps_counter;
} ClientSession;

#define MAX_SESSIONS 1000
ClientSession sessions[MAX_SESSIONS];

// --- Globals ---
SOCKET main_socket = INVALID_SOCKET;
struct sockaddr_in backend_addr_struct;

// --- Helper Functions ---

// Find existing session or return -1
int find_session(struct sockaddr_in *addr) {
  for (int i = 0; i < MAX_SESSIONS; i++) {
    if (sessions[i].active) {
      if (sessions[i].client_addr.sin_addr.s_addr == addr->sin_addr.s_addr &&
          sessions[i].client_addr.sin_port == addr->sin_port) {
        return i;
      }
    }
  }
  return -1;
}

// Create new session
int create_session(struct sockaddr_in *client_addr, const ProxyConfig *config) {
  int free_slot = -1;
  for (int i = 0; i < MAX_SESSIONS; i++) {
    if (!sessions[i].active) {
      free_slot = i;
      break;
    }
  }

  if (free_slot == -1) {
    log_message(LOG_WARNING, "Max sessions reached! Dropping new client.");
    return -1;
  }

  // Create a new socket for this client to talk to backend
  SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (s == INVALID_SOCKET) {
    log_message(LOG_ERROR, "Failed to create backend socket for session");
    return -1;
  }

  // Bind to 0.0.0.0 (random port)
  // Connecting to backend allows us to use send/recv easily and filters
  // incoming
  if (connect(s, (struct sockaddr *)&backend_addr_struct,
              sizeof(backend_addr_struct)) == SOCKET_ERROR) {
    log_message(LOG_ERROR, "Failed to connect backend socket");
    closesocket(s);
    return -1;
  }

  // Set non-blocking might be good, but with select it's file

  sessions[free_slot].active = 1;
  sessions[free_slot].client_addr = *client_addr;
  sessions[free_slot].backend_socket = s;
  sessions[free_slot].last_activity = time(NULL);
  sessions[free_slot].pps_ts = time(NULL);
  sessions[free_slot].pps_counter = 0;

  log_message(LOG_INFO, "New session [%d]: %s:%d", free_slot,
              inet_ntoa(client_addr->sin_addr), ntohs(client_addr->sin_port));
  return free_slot;
}

void cleanup_inactive_sessions(int timeout_sec) {
  time_t now = time(NULL);
  for (int i = 0; i < MAX_SESSIONS; i++) {
    if (sessions[i].active) {
      if (difftime(now, sessions[i].last_activity) > timeout_sec) {
        log_message(LOG_INFO, "Session [%d] timed out", i);
        closesocket(sessions[i].backend_socket);
        sessions[i].active = 0;
      }
    }
  }
}

// --- Main Proxy Loop ---

void start_proxy_server(const ProxyConfig *config) {
#ifdef _WIN32
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    fail_with_message("WSAStartup failed");
  }
#endif

  // Setup Backend Address
  memset(&backend_addr_struct, 0, sizeof(backend_addr_struct));
  backend_addr_struct.sin_family = AF_INET;
  backend_addr_struct.sin_port = htons(config->backend_port);
  backend_addr_struct.sin_addr.s_addr = inet_addr(config->backend_ip);

  // Setup Main Listen Socket
  main_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (main_socket == INVALID_SOCKET) {
    fail_with_message("Failed to create main socket");
  }

  struct sockaddr_in listen_addr;
  listen_addr.sin_family = AF_INET;
  listen_addr.sin_addr.s_addr = INADDR_ANY;
  listen_addr.sin_port = htons(config->listen_port);

  if (bind(main_socket, (struct sockaddr *)&listen_addr, sizeof(listen_addr)) ==
      SOCKET_ERROR) {
    fail_with_message("Bind failed. Is the port already in use?");
  }

  log_message(LOG_INFO,
              "Proxy Shield Started on port %d -> forwarding to %s:%d",
              config->listen_port, config->backend_ip, config->backend_port);

  // Buffer
  char buffer[4096];

  fd_set read_fds;
  struct timeval tv;

  while (1) {
    // Clear and Set FDs
    FD_ZERO(&read_fds);
    FD_SET(main_socket, &read_fds);

    int max_fd = (int)main_socket;

    // Add active session sockets to read_fds
    for (int i = 0; i < MAX_SESSIONS; i++) {
      if (sessions[i].active) {
        FD_SET(sessions[i].backend_socket, &read_fds);
        if ((int)sessions[i].backend_socket > max_fd) {
          max_fd = (int)sessions[i].backend_socket;
        }
      }
    }

    // Timeout 1s to allow cleanup
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    int activity = select(0, &read_fds, NULL, NULL,
                          &tv); // Num fds ignored in generic code, but 0 is
                                // fine for WinSock. max_fd+1 for linux

    if (activity == SOCKET_ERROR) {
#ifdef _WIN32
      log_message(LOG_ERROR, "Select error: %d", WSAGetLastError());
#else
      log_message(LOG_ERROR, "Select error"); // perror would print errno
#endif
      break;
    }

    // --- Handle Main Socket (Client -> Proxy) ---
    if (FD_ISSET(main_socket, &read_fds)) {
      struct sockaddr_in client_addr;
      socklen_t client_len =
          sizeof(client_addr); // socklen_t is safer for portable code
      int len = recvfrom(main_socket, buffer, sizeof(buffer), 0,
                         (struct sockaddr *)&client_addr, &client_len);

      if (len > 0) {
        // Find or Create Session
        int sess_idx = find_session(&client_addr);
        if (sess_idx == -1) {
          sess_idx = create_session(&client_addr, config);
        }

        if (sess_idx != -1) {
          // Rate Limiting Check
          time_t now = time(NULL);
          if (now != sessions[sess_idx].pps_ts) {
            sessions[sess_idx].pps_ts = now;
            sessions[sess_idx].pps_counter = 0;
          }
          sessions[sess_idx].pps_counter++;

          if (sessions[sess_idx].pps_counter > config->max_pps) {
            // silently drop or log
            // log_message(LOG_DEBUG, "Rate limit exceeded for session %d",
            // sess_idx);
            continue;
          }

          // Forward to Backend via Session Socket
          if (send(sessions[sess_idx].backend_socket, buffer, len, 0) ==
              SOCKET_ERROR) {
            // Error sending to backend
          } else {
            sessions[sess_idx].last_activity = time(NULL);
          }
        }
      }
    }

    // --- Handle Session Sockets (Backend -> Proxy) ---
    for (int i = 0; i < MAX_SESSIONS; i++) {
      if (sessions[i].active &&
          FD_ISSET(sessions[i].backend_socket, &read_fds)) {
        int len = recv(sessions[i].backend_socket, buffer, sizeof(buffer), 0);
        if (len > 0) {
          // Forward back to Client via Main Socket
          if (sendto(main_socket, buffer, len, 0,
                     (struct sockaddr *)&sessions[i].client_addr,
                     sizeof(sessions[i].client_addr)) == SOCKET_ERROR) {
            log_message(LOG_ERROR, "Failed to send back to client");
          } else {
            sessions[i].last_activity = time(NULL);
          }
        } else if (len == 0 ||
                   (len ==
                    SOCKET_ERROR)) { // Removed WSA check specifically here for
                                     // brevity, logic mostly same
#ifdef _WIN32
          int err = WSAGetLastError();
          if (err == WSAECONNRESET) {
#else
          // Linux recv returns -1 and errno set. 0 is closure.
          // UDP doesn't really have "closure" but ICMP errors might trigger
          // generic read error
          int err = 0; // dummy
          if (1) {
#endif
            // Backend closed connection or unreachable (ICMP Port Unreachable)
            // Close session? Maybe backend crashed or just restarting.
            // For UDP, connection reset usually means destination unreachable.
            log_message(LOG_WARNING, "Backend unreachable for session %d", i);
            closesocket(sessions[i].backend_socket);
            sessions[i].active = 0;
          }
        }
      }
    }

    cleanup_inactive_sessions(60);
  }

  closesocket(main_socket);
#ifdef _WIN32
  WSACleanup();
#endif
}
