#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

static void trim(char *str) {
    char *end;
    while (*str == ' ' || *str == '\t') str++;
    if (*str == 0) return;
    end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) end--;
    *(end + 1) = 0;
}

static int es_comentario_o_vacio(const char *linea) {
    if (linea[0] == '#' || linea[0] == '\0' || linea[0] == '\n') {
        return 1;
    }
    return 0;
}

static void parsear_signos(const char *valor, Config *cfg) {
    char copia[512];
    char *token;
    
    strncpy(copia, valor, sizeof(copia) - 1);
    cfg->num_signos = 0;
    
    token = strtok(copia, ",");
    while (token != NULL && cfg->num_signos < MAX_SIGNOS) {
        trim(token);
        if (strlen(token) > 0) {
            strncpy(cfg->signos[cfg->num_signos], token, 49);
            cfg->signos[cfg->num_signos][49] = '\0';
            cfg->num_signos++;
        }
        token = strtok(NULL, ",");
    }
}

static void parsear_prediccion(const char *clave, const char *valor, Config *cfg) {
    if (strncmp(clave, "prediccion_", 11) == 0) {
        const char *signo = clave + 11;
        for (int i = 0; i < cfg->num_signos; i++) {
            if (strcasecmp(signo, cfg->signos[i]) == 0) {
                strncpy(cfg->predicciones[i], valor, 255);
                cfg->predicciones[i][255] = '\0';
                if (i >= cfg->num_predicciones) {
                    cfg->num_predicciones = i + 1;
                }
                return;
            }
        }
    }
}

int cargar_configuracion(const char *archivo, Config *cfg) {
    FILE *fp = fopen(archivo, "r");
    if (!fp) {
        fprintf(stderr, "Error: No se pudo abrir %s\n", archivo);
        return -1;
    }
    
    memset(cfg, 0, sizeof(Config));
    cfg->puerto_central = 5000;
    cfg->puerto_horoscopo = 5001;
    cfg->puerto_clima = 5002;
    cfg->tamano_cache = 100;
    cfg->tamano_buffer = 1024;
    cfg->num_hilos_test = 10;
    cfg->consultas_por_hilo = 5;
    
    strcpy(cfg->ip_central, "127.0.0.1");
    strcpy(cfg->ip_horoscopo, "127.0.0.1");
    strcpy(cfg->ip_clima, "127.0.0.1");
    
    char linea[MAX_LINEA];
    
    while (fgets(linea, sizeof(linea), fp)) {
        char clave[64], valor[256];
        
        if (es_comentario_o_vacio(linea)) continue;
        
        if (sscanf(linea, "%63[^=]=%255[^\n]", clave, valor) == 2) {
            trim(clave);
            trim(valor);
            
            if (strcmp(clave, "ip_servidor_central") == 0) {
                strncpy(cfg->ip_central, valor, 63);
            } else if (strcmp(clave, "ip_servidor_horoscopo") == 0) {
                strncpy(cfg->ip_horoscopo, valor, 63);
            } else if (strcmp(clave, "ip_servidor_clima") == 0) {
                strncpy(cfg->ip_clima, valor, 63);
            } else if (strcmp(clave, "puerto_servidor_central") == 0) {
                cfg->puerto_central = atoi(valor);
            } else if (strcmp(clave, "puerto_servidor_horoscopo") == 0) {
                cfg->puerto_horoscopo = atoi(valor);
            } else if (strcmp(clave, "puerto_servidor_clima") == 0) {
                cfg->puerto_clima = atoi(valor);
            } else if (strcmp(clave, "tamano_cache") == 0) {
                cfg->tamano_cache = atoi(valor);
            } else if (strcmp(clave, "tamano_buffer") == 0) {
                cfg->tamano_buffer = atoi(valor);
            } else if (strcmp(clave, "signos") == 0) {
                parsear_signos(valor, cfg);
            } else if (strcmp(clave, "num_hilos") == 0) {
                cfg->num_hilos_test = atoi(valor);
            } else if (strcmp(clave, "consultas_por_hilo") == 0) {
                cfg->consultas_por_hilo = atoi(valor);
            } else {
                parsear_prediccion(clave, valor, cfg);
            }
        }
    }
    
    fclose(fp);
    return 0;
}

void mostrar_configuracion(const Config *cfg) {
    printf("=== Configuracion ===\n");
    printf("IP Central: %s:%d\n", cfg->ip_central, cfg->puerto_central);
    printf("IP Horoscopo: %s:%d\n", cfg->ip_horoscopo, cfg->puerto_horoscopo);
    printf("IP Clima: %s:%d\n", cfg->ip_clima, cfg->puerto_clima);
    printf("Tamano Cache: %d\n", cfg->tamano_cache);
    printf("Tamano Buffer: %d\n", cfg->tamano_buffer);
    if (cfg->num_signos > 0) {
        printf("Signos: ");
        for (int i = 0; i < cfg->num_signos; i++) {
            printf("%s ", cfg->signos[i]);
        }
        printf("\n");
    }
    if (cfg->num_predicciones > 0) {
        printf("Predicciones cargadas: %d\n", cfg->num_predicciones);
    }
    printf("======================\n");
    fflush(stdout);
}

const char* obtener_prediccion(const Config *cfg, const char *signo) {
    for (int i = 0; i < cfg->num_signos; i++) {
        if (strcasecmp(signo, cfg->signos[i]) == 0) {
            if (strlen(cfg->predicciones[i]) > 0) {
                return cfg->predicciones[i];
            }
            return "Prediccion no disponible para este signo.";
        }
    }
    return "Signo no reconocido.";
}
