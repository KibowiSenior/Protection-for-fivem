# UDP Proxy Shield Setup Guide

Welcome to your high-performance UDP Proxy Shield for FiveM.
This tool sits in front of your real game server, hiding its IP and protecting it from basic attacks.

## 🛡️ Features
- **IP Protection**: Players connect to this proxy; your real backend IP remains hidden.
- **DDoS Mitigation**: Built-in rate limiting drops packets from spammers.
- **Cross-Platform**: Runs on **Windows** (local PC/VPS) and **Linux** (Codespaces/VPS).
- **Session Management**: Correctly maps multiple players to the backend server.

---

## ⚙️ Configuration
The configuration is currently located in `config.c`. Check this file before compiling!

| Setting | Default | Description |
| :--- | :--- | :--- |
| `MAX_PPS` | `1000` | Max packets per second allowed per player IP. |
| `BLOCK_DURATION` | `60` | How long (seconds) to ignore a spammer. |
| `config->listen_port` | `30120` | Port players connect to (Proxy Port). |
| `config->backend_ip` | `127.0.0.1` | **IMPORTANT**: IP of your REAL FiveM Server. |
| `config->backend_port` | `30121` | Port of your REAL FiveM Server. |

> **Note**: If your FiveM server is on a different machine, change `127.0.0.1` to that machine's Public IP.

---

## 🚀 How to Start

### Option A: Windows
1. **Prerequisites**: Ensure you have MinGW (GCC) installed.
   ```powershell
   gcc -O2 -o output.exe main.c proxy.c config.c utils.c -lws2_32
   .\output.exe
   ```

### Option B: Linux (C server / VPS)
1. **Open Terminal**.
2. **Compile**:
   ```bash
   gcc -O2 -o proxy_shield main.c proxy.c config.c utils.c
   ```
3. **Run**:
   ```bash
   ./proxy_shield
   ```

---

## 🎮 Connecting
1. Start your **Real FiveM Server** on port `30121`.
2. Start this **Proxy Shield** on port `30120`.
3. Players connect to: `connect <Proxy_IP>:30120`

## ❓ Troubleshooting
- **"Bind failed"**: The port `30120` is already in use. Ensure no other FiveM server is running on that specific port.
- **Players can't connect**:
  - Check `backend_ip` in `config.c`.
  - Ensure Firewall (Windows/Linux) allows entering traffic on UDP 30120.



