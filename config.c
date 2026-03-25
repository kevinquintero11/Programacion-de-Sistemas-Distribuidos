#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "config.h"

static void trim(char *str) {
    char *end;
    while (*str == ' ' || *str == '\t') str++;
    if (*str == 0) return;
    end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) end--;
    *(end + 1) = 0;
}

static int obtener_valor_int(FILE *fp, const char *key_buscado, int defecto) {
    char linea[512];
    rewind(fp);
    
    while (fgets(linea, sizeof(linea), fp)) {
        trim(linea);
        if (linea[0] == '\0' || linea[0] == '#') continue;
        
        char *eq = strchr(linea, '=');
        if (eq) {
            *eq = '\0';
            trim(linea);
            if (strcmp(linea, key_buscado) == 0) {
                trim(eq + 1);
                return atoi(eq + 1);
            }
        }
    }
    return defecto;
}

static void obtener_valor_string(FILE *fp, const char *key_buscado, char *dest, int tamano, const char *defecto) {
    char linea[512];
    strcpy(dest, defecto);
    rewind(fp);
    
    while (fgets(linea, sizeof(linea), fp)) {
        trim(linea);
        if (linea[0] == '\0' || linea[0] == '#') continue;
        
        char *eq = strchr(linea, '=');
        if (eq) {
            *eq = '\0';
            trim(linea);
            if (strcmp(linea, key_buscado) == 0) {
                trim(eq + 1);
                strncpy(dest, eq + 1, tamano - 1);
                dest[tamano - 1] = '\0';
                return;
            }
        }
    }
}

int cargar_configuracion(const char *config_file, const char *signos_file, const char *pronosticos_file, Config *cfg) {
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
    
    FILE *fp = fopen(config_file, "r");
    if (fp) {
        obtener_valor_string(fp, "ip_servidor_central", cfg->ip_central, 64, "127.0.0.1");
        obtener_valor_string(fp, "ip_servidor_horoscopo", cfg->ip_horoscopo, 64, "127.0.0.1");
        obtener_valor_string(fp, "ip_servidor_clima", cfg->ip_clima, 64, "127.0.0.1");
        
        cfg->puerto_central = obtener_valor_int(fp, "puerto_servidor_central", 5000);
        cfg->puerto_horoscopo = obtener_valor_int(fp, "puerto_servidor_horoscopo", 5001);
        cfg->puerto_clima = obtener_valor_int(fp, "puerto_servidor_clima", 5002);
        cfg->tamano_cache = obtener_valor_int(fp, "tamano_cache", 100);
        cfg->tamano_buffer = obtener_valor_int(fp, "tamano_buffer", 1024);
        cfg->num_hilos_test = obtener_valor_int(fp, "num_hilos", 10);
        cfg->consultas_por_hilo = obtener_valor_int(fp, "consultas_por_hilo", 5);
        
        fclose(fp);
    }
    
    if (signos_file) {
        fp = fopen(signos_file, "r");
        if (fp) {
            char linea[512];
            cfg->num_signos = 0;
            while (fgets(linea, sizeof(linea), fp) && cfg->num_signos < MAX_SIGNOS) {
                trim(linea);
                if (linea[0] == '\0' || linea[0] == '#') continue;
                
                char *pipe = strchr(linea, '|');
                if (pipe) {
                    *pipe = '\0';
                    trim(linea);
                    trim(pipe + 1);
                    strncpy(cfg->signos[cfg->num_signos], linea, 49);
                    strncpy(cfg->predicciones[cfg->num_signos], pipe + 1, 255);
                    cfg->num_signos++;
                }
            }
            fclose(fp);
        }
    }
    
    if (pronosticos_file) {
        fp = fopen(pronosticos_file, "r");
        if (fp) {
            char linea[512];
            cfg->num_pronosticos = 0;
            while (fgets(linea, sizeof(linea), fp) && cfg->num_pronosticos < MAX_PRONOSTICOS) {
                trim(linea);
                if (linea[0] == '\0' || linea[0] == '#') continue;
                strncpy(cfg->pronosticos[cfg->num_pronosticos], linea, 511);
                cfg->num_pronosticos++;
            }
            fclose(fp);
        }
    }
    
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

const char* obtener_pronostico(const Config *cfg, int index) {
    if (index >= 0 && index < cfg->num_pronosticos) {
        return cfg->pronosticos[index];
    }
    return "Pronostico no disponible.";
}
