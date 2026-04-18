# Sistema de Consulta de Horóscopo y Clima

Sistema cliente-servidor distribuido que permite consultar horóscopos y predicciones del clima mediante sockets TCP con arquitectura de microservices.

## Arquitectura

```
┌─────────────┐         ┌──────────────────┐         ┌──────────────────┐
│   Cliente   │ ──────> │ Servidor Central │ ──────> │ Serv. Horóscopo  │
│             │ <────── │                  │ <────── │                  │
└─────────────┘         │   ┌──────────┐   │         └──────────────────┘
                        │   │  Cache   │   │
                        │   │  (100)   │   │
                        │   └──────────┘   │
                        │                  │         ┌──────────────────┐
                        │                  │ ──────> │  Serv. Clima     │
                        │                  │ <────── │                  │
                        └──────────────────┘         └──────────────────┘
```

## Estructura del Proyecto

```
Lab1/
├── servidor_central/
│   ├── servidor_central.c
│   ├── config.h
│   ├── config.c
│   └── config.conf
│
├── servidor_horoscopo/
│   ├── servidor_horoscopo.c
│   ├── config.h
│   ├── config.c
│   ├── config.conf
│   └── signos.txt
│
├── servidor_clima/
│   ├── servidor_clima.c
│   ├── config.h
│   ├── config.c
│   ├── config.conf
│   └── pronosticos.txt
│
├── cliente/
│   ├── cliente.c
│   ├── config.h
│   ├── config.c
│   ├── config.conf
│   ├── signos.txt
│   └── cliente_test/
│       ├── cliente_test.c
│       ├── config.h
│       ├── config.c
│       ├── config.conf
│       └── signos.txt
│
├── Makefile
└── README.md
```

## Archivos de Datos

| Archivo | Ubicación | Descripción |
|---------|----------|-------------|
| `signos.txt` | servidor_horoscopo/ | Signos zodiacales con predicciones (signo\|prediccion) |
| `signos.txt` | cliente/cliente_test/ | Lista de signos (solo nombres) |
| `pronosticos.txt` | servidor_clima/ | Pronósticos del clima (uno por línea) |

### Formato signos.txt (servidor_horoscopo)

```
aries|El fuego de Aries te impulsa a tomar la iniciativa hoy.
tauro|La perseverancia de Tauro te traera recompensas tangibles.
...
```

### Formato signos.txt (cliente/cliente_test)

```
aries
tauro
geminis
...
```

### Formato pronosticos.txt

```
Día soleado con cielos despejados. Perfecto para actividades al aire libre.
Nublado con posibilidad de lluvias aisladas en la tarde. Lleva un paraguas.
...
```

## Configuración

Cada componente tiene su propio `config.conf` con los parámetros necesarios.

### Servidor Central

```bash
# === PUERTOS ===
puerto_servidor_central=5000
puerto_servidor_horoscopo=5001
puerto_servidor_clima=5002

# === IPs SERVIDORES SECUNDARIOS ===
ip_servidor_horoscopo=127.0.0.1
ip_servidor_clima=127.0.0.1

# === CACHE ===
tamano_cache=100

# === BUFFER ===
tamano_buffer=1024
```

### Servidor Horóscopo

```bash
# === PUERTOS ===
puerto_servidor_horoscopo=5001

# === BUFFER ===
tamano_buffer=1024
```

### Servidor Clima

```bash
# === PUERTOS ===
puerto_servidor_clima=5002

# === BUFFER ===
tamano_buffer=1024
```

### Cliente

```bash
# === SERVIDOR CENTRAL ===
ip_servidor_central=127.0.0.1
puerto_servidor_central=5000

# === BUFFER ===
tamano_buffer=1024
```

### Cliente Test

```bash
# === SERVIDOR CENTRAL ===
ip_servidor_central=127.0.0.1
puerto_servidor_central=5000

# === BUFFER ===
tamano_buffer=1024

# === TEST ===
num_hilos=10
consultas_por_hilo=5
```

## Compilación

```bash
make all      # Compila todos los componentes
make clean    # Elimina binarios
```

**Requisitos:**
- Compilador GCC
- Biblioteca POSIX threads (`-pthread`)
- Sistema operativo: Linux/Unix

## Ejecución

### 1. Compilar

```bash
make all
```

### 2. Iniciar servidores (en orden)

```bash
cd servidor_central && ./sc &
cd ../servidor_horoscopo && ./sh &
cd ../servidor_clima && ./sp &
```

### 3. Ejecutar cliente

```bash
cd cliente && ./cliente <signo> <dd/mm/yyyy>

# Ejemplo:
./cliente leo 12/08/1999
```

### 4. Test de estrés (cliente_test)

```bash
cd cliente/cliente_test && ./cliente_test
```

## Protocolo de Comunicación

### Formato de Mensajes

```
<signo>|<fecha>
```

### Ejemplo

**Cliente → Servidor Central:**
```
leo|12/08/1999
```

**Servidor Central → Cliente:**
```
Horóscopo leo: La creatividad de Leo brilla con fuerza hoy. Expresate libremente.
Clima 12/08/1999: Temperaturas cálidas, alcanzando los 28°C. Hidrátate adecuadamente.
```

### Flujo de datos

1. Cliente envía `signo|fecha` al Servidor Central
2. Servidor Central consulta en paralelo:
   - Servidor Horóscopo (pasa el signo)
   - Servidor Clima (pasa la fecha)
3. Servidor Central combina respuestas y las devuelve al cliente

## Detalles Técnicos

### Sockets

- **Tipo:** TCP (SOCK_STREAM)
- **Familia:** IPv4 (AF_INET)
- **Puerto cliente:** Asignado dinámicamente por el sistema
- **Backlog:** 5 conexiones en cola

### Hilos y Concurrencia

- Cada servidor crea un hilo por cada cliente conectado
- `pthread_detach()` para liberar recursos automáticamente
- Mutex (`pthread_mutex_t`) protege el acceso a la caché

### Caché

- **Tamaño:** Configurable (default: 100)
- **Clave:** `signo\|fecha`
- **Thread-safe:** Sí, mediante mutex

### Selección de Pronósticos

Los pronósticos del clima se seleccionan aleatoriamente usando `rand()` con semilla basada en `time(NULL) + strlen(fecha)`.
