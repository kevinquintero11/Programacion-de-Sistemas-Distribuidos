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

## Componentes

| Componente | Archivo | Descripción |
|------------|---------|-------------|
| Cliente | `cliente.c` | Aplicación CLI para consultas |
| Cliente Test | `cliente_test.c` | Utilidad de pruebas con hilos |
| Servidor Central | `servidor_central.c` | Orquestador con caché |
| Servidor Horóscopo | `servidor_horoscopo.c` | Genera predicciones zodiacales |
| Servidor Clima | `servidor_clima.c` | Genera pronósticos del tiempo |
| Configuración | `config.h`, `config.c`, `config.conf` | Gestión de configuración |

## Archivos de Datos

El sistema utiliza archivos de texto externos para los datos:

| Archivo | Descripción |
|---------|-------------|
| `config.conf` | Configuración general del sistema |
| `signos.txt` | Signos zodiacales y predicciones (formato: signo\|prediccion) |
| `pronosticos.txt` | Pronósticos del clima (uno por línea) |

### Formato de signos.txt

```
aries|El fuego de Aries te impulsa a tomar la iniciativa hoy.
tauro|La perseverancia de Tauro te traera recompensas tangibles.
...
```

### Formato de pronosticos.txt

```
Día soleado con cielos despejados. Perfecto para actividades al aire libre.
Nublado con posibilidad de lluvias aisladas en la tarde. Lleva un paraguas.
...
```

## Configuración

Todos los parámetros del sistema se gestionan desde el archivo `config.conf`:

```bash
# === DIRECCIONES IP ===
ip_servidor_central=127.0.0.1
ip_servidor_horoscopo=127.0.0.1
ip_servidor_clima=127.0.0.1

# === PUERTOS ===
puerto_servidor_central=5000
puerto_servidor_horoscopo=5001
puerto_servidor_clima=5002

# === CACHE ===
tamano_cache=100

# === BUFFER ===
tamano_buffer=1024

# === NUMERO DE HILOS Y CONSULTAS (cliente_test) ===
num_hilos=10
consultas_por_hilo=5
```

### Parámetros configurables

| Parámetro | Descripción | Valor por defecto |
|-----------|-------------|-------------------|
| `ip_servidor_central` | IP del servidor central | 127.0.0.1 |
| `ip_servidor_horoscopo` | IP del servidor de horóscopo | 127.0.0.1 |
| `ip_servidor_clima` | IP del servidor de clima | 127.0.0.1 |
| `puerto_servidor_central` | Puerto TCP del servidor central | 5000 |
| `puerto_servidor_horoscopo` | Puerto TCP del servidor de horóscopo | 5001 |
| `puerto_servidor_clima` | Puerto TCP del servidor de clima | 5002 |
| `tamano_cache` | Número máximo de entradas en caché | 100 |
| `tamano_buffer` | Tamaño del buffer de red (bytes) | 1024 |
| `num_hilos` | Hilos en cliente_test | 10 |
| `consultas_por_hilo` | Consultas por hilo en cliente_test | 5 |

**Nota:** Los comentarios (líneas con `#`) y espacios en blanco son ignorados.

## Compilación

```bash
make all      # Compila todos los componentes
make clean    # Elimina binarios
```

**Requisitos:**
- Compilador GCC
- Biblioteca POSIX threads (`-pthread`)
- Sistema operativo: Linux/Unix

**Flags utilizados:**
- `-Wall`: Warnings habilitados
- `-pthread`: Soporte para hilos POSIX
- `-g`: Depuración

## Ejecución

### 1. Iniciar servidores (en terminales separadas)

```bash
./servidor_horoscopo    # Terminal 1
./servidor_clima        # Terminal 2
./servidor_central      # Terminal 3
```

### 2. Ejecutar cliente

```bash
./cliente <signo> <dd/mm/yyyy>
```

**Ejemplo:**
```bash
./cliente leo 12/08/1999
```

### 3. Test de estrés

```bash
./cliente_test
```

Genera consultas concurrentes según los valores en `config.conf`.

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
>>>>>>> 6b41435 (update de archivos con datos)
2. Servidor Central consulta:
   - Servidor Horóscopo
   - Servidor Clima
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
- **Clave:** `signo|fecha`
- **Thread-safe:** Sí, mediante mutex

### Predicciones

Los signos zodiacales y predicciones se cargan desde `signos.txt`, y los pronósticos del clima desde `pronosticos.txt`. Las predicciones se seleccionan aleatoriamente usando `rand()` con semilla basada en `time(NULL) + strlen(dato)`.

## Estructura del Código

```
cliente.c              - Cliente principal (monohilo)
cliente_test.c         - Cliente de pruebas (hilos configurables)
servidor_central.c     - Orquestador con caché
servidor_horoscopo.c   - Microservicio de horóscopo
servidor_clima.c       - Microservicio de clima
config.h               - Header de configuración
config.c               - Parser de configuración
config.conf            - Archivo de configuración
