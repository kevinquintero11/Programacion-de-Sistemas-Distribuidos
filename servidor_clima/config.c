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

int cargar_configuracion(const char *config_file, const char *pronosticos_file, Config *cfg) {
    memset(cfg, 0, sizeof(Config));
    
    FILE *fp = fopen(config_file, "r");
    if (!fp) {
        fprintf(stderr, "Error: No se pudo abrir %s\n", config_file);
        return -1;
    }
    
    cfg->puerto_clima = obtener_valor_int(fp, "puerto_servidor_clima", 5002);
    cfg->tamano_buffer = obtener_valor_int(fp, "tamano_buffer", 1024);
    
    strcpy(cfg->ip_clima, "127.0.0.1");
    
    fclose(fp);
    
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

int formato_es_valido(const char* pronostico) {
    // 1. Chequeo básico: que no sea nulo
    if (pronostico == NULL) {
        return 0;
    }
    
    // 2. Validar que la longitud sea exactamente 10 ("dd/mm/aaaa")
    if (strlen(pronostico) != 10) {
        return 0;
    }

    // 3. Validar que haya barras '/' en las posiciones correctas
    if (pronostico[2] != '/' || pronostico[5] != '/') {
        return 0;
    }

    // 4. Validar que el resto de los caracteres sean dígitos numéricos
    for (int i = 0; i < 10; i++) {
        // Saltamos las posiciones donde sabemos que están las barras
        if (i == 2 || i == 5) {
            continue;
        }
        
        // Si algún otro caracter no es un número, el formato es inválido
        if (!isdigit((unsigned char)pronostico[i])) {
            return 0;
        }
    }

    return 1; 
}

const char* obtener_pronostico(const Config *cfg, int index) {
    if (index >= 0 && index < cfg->num_pronosticos) {
        return cfg->pronosticos[index];
    }
    return "Pronostico no disponible.";
}

void mostrar_configuracion(const Config *cfg) {
    printf("Puerto Clima: %d\n", cfg->puerto_clima);
    printf("Tamano Buffer: %d\n", cfg->tamano_buffer);
    fflush(stdout);
}
