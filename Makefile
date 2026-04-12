CC = gcc
CFLAGS = -Wall -pthread -g
UTILS_PATH = utils

all: cliente/cliente cliente/cliente_test servidor_central/sc servidor_horoscopo/sh servidor_clima/sp

config.o: $(UTILS_PATH)/config.c $(UTILS_PATH)/config.h
	$(CC) $(CFLAGS) -I$(UTILS_PATH) -c $(UTILS_PATH)/config.c

cliente/cliente: cliente/cliente.c config.o
	$(CC) $(CFLAGS) -I$(UTILS_PATH) -o cliente/cliente cliente/cliente.c config.o

cliente/cliente_test: cliente/cliente_test.c config.o
	$(CC) $(CFLAGS) -I$(UTILS_PATH) -o cliente/cliente_test cliente/cliente_test.c config.o

servidor_central/sc: servidor_central/servidor_central.c config.o
	$(CC) $(CFLAGS) -I$(UTILS_PATH) -o servidor_central/sc servidor_central/servidor_central.c config.o

servidor_horoscopo/sh: servidor_horoscopo/servidor_horoscopo.c config.o
	$(CC) $(CFLAGS) -I$(UTILS_PATH) -o servidor_horoscopo/sh servidor_horoscopo/servidor_horoscopo.c config.o

servidor_clima/sp: servidor_clima/servidor_clima.c config.o
	$(CC) $(CFLAGS) -I$(UTILS_PATH) -o servidor_clima/sp servidor_clima/servidor_clima.c config.o

clean:
	rm -f cliente/cliente cliente/cliente_test servidor_central/sc servidor_horoscopo/sh servidor_clima/sp *.o

.PHONY: all clean
