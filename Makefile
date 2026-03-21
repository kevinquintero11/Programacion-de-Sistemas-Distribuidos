# Makefile para el sistema de sockets

CC = gcc
CFLAGS = -Wall -pthread -g
TARGETS = cliente servidor_central servidor_horoscopo servidor_clima

all: $(TARGETS)

cliente: cliente.c
	$(CC) $(CFLAGS) -o cliente cliente.c

servidor_central: servidor_central.c
	$(CC) $(CFLAGS) -o servidor_central servidor_central.c

servidor_horoscopo: servidor_horoscopo.c
	$(CC) $(CFLAGS) -o servidor_horoscopo servidor_horoscopo.c

servidor_clima: servidor_clima.c
	$(CC) $(CFLAGS) -o servidor_clima servidor_clima.c

clean:
	rm -f $(TARGETS) *.o

.PHONY: all clean