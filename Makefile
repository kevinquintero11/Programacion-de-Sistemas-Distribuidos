CC = gcc
CFLAGS = -Wall -pthread -g

TARGETS = cliente cliente_test servidor_central servidor_horoscopo servidor_clima

all: $(TARGETS)

config.o: config.c config.h
	$(CC) $(CFLAGS) -c config.c

cliente: cliente.c config.o
	$(CC) $(CFLAGS) -o cliente cliente.c config.o

cliente_test: cliente_test.c config.o
	$(CC) $(CFLAGS) -o cliente_test cliente_test.c config.o

servidor_central: servidor_central.c config.o
	$(CC) $(CFLAGS) -o servidor_central servidor_central.c config.o

servidor_horoscopo: servidor_horoscopo.c config.o
	$(CC) $(CFLAGS) -o servidor_horoscopo servidor_horoscopo.c config.o

servidor_clima: servidor_clima.c config.o
	$(CC) $(CFLAGS) -o servidor_clima servidor_clima.c config.o

clean:
	rm -f $(TARGETS) *.o

.PHONY: all clean
