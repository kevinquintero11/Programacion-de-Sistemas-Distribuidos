CC = gcc
CFLAGS = -Wall -pthread -g

TARGETS = cliente cliente_test servidor_central servidor_horoscopo servidor_clima

all: $(TARGETS)

json.o: json.c json.h
	$(CC) $(CFLAGS) -c json.c

config.o: config.c config.h json.o
	$(CC) $(CFLAGS) -c config.c

cliente: cliente.c config.o json.o
	$(CC) $(CFLAGS) -o cliente cliente.c config.o json.o

cliente_test: cliente_test.c config.o json.o
	$(CC) $(CFLAGS) -o cliente_test cliente_test.c config.o json.o

servidor_central: servidor_central.c config.o json.o
	$(CC) $(CFLAGS) -o servidor_central servidor_central.c config.o json.o

servidor_horoscopo: servidor_horoscopo.c config.o json.o
	$(CC) $(CFLAGS) -o servidor_horoscopo servidor_horoscopo.c config.o json.o

servidor_clima: servidor_clima.c config.o json.o
	$(CC) $(CFLAGS) -o servidor_clima servidor_clima.c config.o json.o

clean:
	rm -f $(TARGETS) *.o

.PHONY: all clean
