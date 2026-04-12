CC = gcc
CFLAGS = -Wall -pthread -g

all: cliente/cliente cliente/cliente_test/cliente_test servidor_central/sc servidor_horoscopo/sh servidor_clima/sp

cliente/cliente: cliente/cliente.c cliente/config.c
	$(CC) $(CFLAGS) -o cliente/cliente cliente/cliente.c cliente/config.c

cliente/cliente_test/cliente_test: cliente/cliente_test/cliente_test.c cliente/cliente_test/config.c
	$(CC) $(CFLAGS) -o cliente/cliente_test/cliente_test cliente/cliente_test/cliente_test.c cliente/cliente_test/config.c

servidor_central/sc: servidor_central/servidor_central.c servidor_central/config.c
	$(CC) $(CFLAGS) -o servidor_central/sc servidor_central/servidor_central.c servidor_central/config.c

servidor_horoscopo/sh: servidor_horoscopo/servidor_horoscopo.c servidor_horoscopo/config.c
	$(CC) $(CFLAGS) -o servidor_horoscopo/sh servidor_horoscopo/servidor_horoscopo.c servidor_horoscopo/config.c

servidor_clima/sp: servidor_clima/servidor_clima.c servidor_clima/config.c
	$(CC) $(CFLAGS) -o servidor_clima/sp servidor_clima/servidor_clima.c servidor_clima/config.c

clean:
	rm -f cliente/cliente cliente/cliente_test/cliente_test servidor_central/sc servidor_horoscopo/sh servidor_clima/sp

.PHONY: all clean
