# Sistema de Consulta de Horoscopo y Clima con Java RMI

Migracion de `Lab1` a Java usando invocacion de metodos remota (RMI), manteniendo la misma organizacion general del proyecto: cliente, servidor central y dos servidores secundarios.

## Arquitectura

```
Cliente RMI -> Servidor Central RMI -> Servidor Horoscopo RMI
                                  -> Servidor Clima RMI
```

- El cliente invoca al servidor central con `signo` y `fecha`.
- El servidor central consulta en paralelo a los servicios de horoscopo y clima.
- El servidor central mantiene cache en memoria por clave `signo|fecha`.
- Cada servidor publica su servicio en su propio registro RMI.

## Estructura

```
Lab2/
|-- servidor_central/
|   |-- ServidorCentral.java
|   |-- ConsultaCentralRemote.java
|   |-- ConsultaRequest.java
|   |-- ConsultaResponse.java
|   |-- HoroscopoRemote.java
|   |-- ClimaRemote.java
|   |-- Config.java
|   `-- config.conf
|-- servidor_horoscopo/
|   |-- ServidorHoroscopo.java
|   |-- HoroscopoRemote.java
|   |-- Config.java
|   |-- config.conf
|   `-- signos.txt
|-- servidor_clima/
|   |-- ServidorClima.java
|   |-- ClimaRemote.java
|   |-- Config.java
|   |-- config.conf
|   `-- pronosticos.txt
|-- cliente/
|   |-- Cliente.java
|   |-- ConsultaCentralRemote.java
|   |-- ConsultaRequest.java
|   |-- ConsultaResponse.java
|   |-- Config.java
|   |-- config.conf
|   `-- cliente_test/
|       |-- ClienteTest.java
|       |-- ConsultaCentralRemote.java
|       |-- ConsultaRequest.java
|       |-- ConsultaResponse.java
|       |-- Config.java
|       |-- config.conf
|       `-- signos.txt
|-- Makefile
`-- README.md
```

## Configuracion

Se conservaron las claves principales de `Lab1` para que la migracion sea directa.

### Servidor Central

```conf
ip_servidor_horoscopo=127.0.0.1
ip_servidor_clima=127.0.0.1
puerto_servidor_central=5000
puerto_servidor_horoscopo=5001
puerto_servidor_clima=5002
tamano_cache=100
tamano_buffer=1024
```

### Servidor Horoscopo

```conf
puerto_servidor_horoscopo=5001
tamano_buffer=1024
```

### Servidor Clima

```conf
puerto_servidor_clima=5002
tamano_buffer=1024
```

### Cliente

```conf
ip_servidor_central=127.0.0.1
puerto_servidor_central=5000
tamano_buffer=1024
```

### Cliente Test

```conf
ip_servidor_central=127.0.0.1
puerto_servidor_central=5000
num_hilos=10
consultas_por_hilo=5
tamano_buffer=1024
```

## Compilacion

Requiere JDK 8 o superior con `javac` y `java` disponibles.

```bash
make all
make clean
```

## Ejecucion

Iniciar los servidores en este orden, en terminales separadas:

```bash
cd servidor_horoscopo && java ServidorHoroscopo
cd servidor_clima && java ServidorClima
cd servidor_central && java ServidorCentral
```

Ejecutar cliente:

```bash
cd cliente && java Cliente leo 12/08/1999
```

Ejecutar test concurrente:

```bash
cd cliente/cliente_test && java ClienteTest
```

## Flujo de la consulta

1. El cliente crea `ConsultaRequest(signo, fecha)`.
2. El servidor central busca la clave en cache.
3. Si no existe, consulta a los servicios remotos de horoscopo y clima en paralelo.
4. Combina ambas respuestas en `ConsultaResponse`.
5. Guarda el resultado en cache y lo devuelve al cliente.

## Nota

Los registros RMI se crean desde cada servidor, por lo que no hace falta ejecutar `rmiregistry` manualmente.
