JAVAC = javac

CLIENTE_SRC = $(wildcard cliente/*.java)
CLIENTE_TEST_SRC = $(wildcard cliente/cliente_test/*.java)
CENTRAL_SRC = $(wildcard servidor_central/*.java)
HOROSCOPO_SRC = $(wildcard servidor_horoscopo/*.java)
CLIMA_SRC = $(wildcard servidor_clima/*.java)

all: servidor_horoscopo servidor_clima servidor_central cliente cliente_test

servidor_horoscopo:
	$(JAVAC) $(HOROSCOPO_SRC)

servidor_clima:
	$(JAVAC) $(CLIMA_SRC)

servidor_central:
	$(JAVAC) $(CENTRAL_SRC)

cliente:
	$(JAVAC) $(CLIENTE_SRC)

cliente_test:
	$(JAVAC) $(CLIENTE_TEST_SRC)

clean:
	rm -f cliente/*.class cliente/cliente_test/*.class servidor_central/*.class servidor_horoscopo/*.class servidor_clima/*.class

.PHONY: all clean servidor_horoscopo servidor_clima servidor_central cliente cliente_test
