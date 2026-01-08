CC = gcc
CFLAGS = -O2 -Wall
LDFLAGS = -lws2_32

output.exe: main.c proxy.c config.c utils.c
	$(CC) $(CFLAGS) -o output.exe main.c proxy.c config.c utils.c $(LDFLAGS)

clean:
	del output.exe
