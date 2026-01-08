@echo off
echo [INFO] Checking for GCC...
where gcc >nul 2>nul
if %errorlevel% neq 0 (
    echo [ERROR] GCC is not found! Please install MinGW or ensure GCC is in your PATH.
    echo [TIP] You can install it via Chocolatey: choco install mingw
    pause
    exit /b
)

echo [INFO] Compiling Proxy Shield...
gcc -O2 -Wall -o proxy_shield.exe main.c proxy.c config.c utils.c -lws2_32
if %errorlevel% neq 0 (
    echo [ERROR] Compilation Failed!
    pause
    exit /b
)

echo [INFO] Compilation Successful!
echo [INFO] Starting Proxy Shield...
proxy_shield.exe
pause
