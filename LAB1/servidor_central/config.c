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

int cargar_configuracion(const char *config_file, void *unused, Config *cfg) {
    (void)unused;
    memset(cfg, 0, sizeof(Config));
    
    FILE *fp = fopen(config_file, "r");
    if (!fp) {
        fprintf(stderr, "Error: No se pudo abrir %s\n", config_file);
        return -1;
    }
    
    cfg->puerto_central = obtener_valor_int(fp, "puerto_servidor_central", 5000);
    cfg->puerto_horoscopo = obtener_valor_int(fp, "puerto_servidor_horoscopo", 5001);
    cfg->puerto_clima = obtener_valor_int(fp, "puerto_servidor_clima", 5002);
    cfg->tamano_cache = obtener_valor_int(fp, "tamano_cache", 100);
    cfg->tamano_buffer = obtener_valor_int(fp, "tamano_buffer", 1024);
    
    obtener_valor_string(fp, "ip_servidor_horoscopo", cfg->ip_horoscopo, 64, "127.0.0.1");
    obtener_valor_string(fp, "ip_servidor_clima", cfg->ip_clima, 64, "127.0.0.1");
    
    fclose(fp);
    return 0;
}

void mostrar_configuracion(const Config *cfg) {
    printf("=== Configuracion ===\n");
    printf("Puerto Central: %d\n", cfg->puerto_central);
    printf("Puerto Horoscopo: %d\n", cfg->puerto_horoscopo);
    printf("Puerto Clima: %d\n", cfg->puerto_clima);
    printf("Tamano Cache: %d\n", cfg->tamano_cache);
    printf("Tamano Buffer: %d\n", cfg->tamano_buffer);
    printf("======================\n");
    fflush(stdout);
}
