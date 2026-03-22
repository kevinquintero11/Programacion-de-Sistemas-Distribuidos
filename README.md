# Sistema de Consulta de Horóscopo y Clima

Sistema cliente-servidor distribuido que permite consultar horóscopos y predicciones del clima mediante sockets TCP con arquitectura de microservices.

## Arquitectura

```
┌─────────────┐         ┌──────────────────┐         ┌──────────────────┐
│   Cliente   │ ──────> │ Servidor Central │ ──────> │ Serv. Horóscopo │
│  (puerto    │ <────── │    (puerto 5000) │ <────── │  (puerto 5001)  │
│   cliente)  │         │                  │         │                  │
└─────────────┘         │   ┌──────────┐   │         └──────────────────┘
                        │   │  Cache   │   │
                        │   │  (100)   │   │
                        │   └──────────┘   │
                        │                  │         ┌──────────────────┐
                        │                  │ ──────> │  Serv. Clima    │
                        │                  │ <────── │  (puerto 5002)  │
                        └──────────────────┘         └──────────────────┘
```

## Componentes

| Componente | Archivo | Puerto | Descripción |
|------------|---------|--------|-------------|
| Cliente | `cliente.c` | - | Aplicación CLI para consultas |
| Cliente Test | `cliente_test.c` | - | Utilidad de pruebas con hilos |
| Servidor Central | `servidor_central.c` | 5000 | Orquestador con caché |
| Servidor Horóscopo | `servidor_horoscopo.c` | 5001 | Genera predicciones zodiacales |
| Servidor Clima | `servidor_clima.c` | 5002 | Genera pronósticos del tiempo |

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
./cliente <signo> <fecha>
```

**Ejemplo:**
```bash
./cliente leo 12/08/1999
```

**Signos válidos:**
`leo`, `aries`, `tauro`, `cancer`, `virgo`

**Formato de fecha:** `dd/mm/yyyy`

### 3. Test de estrés

```bash
./cliente_test
```

Genera 50 consultas concurrentes (10 hilos x 5 consultas) para validar el sistema bajo carga.

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
Horóscopo leo: Hoy será un día excepcional para ti. Las estrellas alinean tu camino hacia el éxito.
Clima 12/08/1999: Temperaturas cálidas, alcanzando los 28°C. Hidrátate adecuadamente.
```

### Flujo de datos

1. Cliente envía `signo|datum` al Servidor Central
2. Servidor Central consulta simultáneamente:
   - Servidor Horóscopo (puerto 5001)
   - Servidor Clima (puerto 5002)
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

- **Tamaño:** 100 entradas
- **Clave:** `signo羽毛a`
- **Políticas:** FIFO simple (sin eviction avanzada)
- **Thread-safe:** Sí, mediante mutex

### Predicciones

| Servidor | Pool de predicciones |
|----------|---------------------|
| Horóscopo | 10 frases astrológicas |
| Clima | 10 escenarios meteorológicos |

Las predicciones se seleccionan aleatoriamente usando `rand()` con semilla basada en `time(NULL) + strlen(dato)`.

## Estructura del Código

```
cliente.c              - Cliente principal (monohilo)
cliente_test.c         - Cliente de pruebas (10 hilos)
servidor_central.c    - Orquestador con caché
servidor_horoscopo.c  - Microservicio de horóscopo
servidor_clima.c      - Microservicio de clima
Makefile              - Script de compilación
```

## Manejo de Errores

| Situación | Comportamiento |
|-----------|----------------|
| Socket no creado | `perror()` + exit(1) |
| bind() falla | `perror()` + exit(1) |
| connect() falla | `perror()` + continúa |
| recv() falla | Cierra conexión + retorna |
| Servidor no disponible | Retorna mensaje de error |

## Limitaciones

- Caché limitada a 100 entradas (sin rotación)
- Sin persistencia de datos
- Signos zodiacales hardcodeados
- No hay autenticación
- Comunicación local (127.0.0.1)
