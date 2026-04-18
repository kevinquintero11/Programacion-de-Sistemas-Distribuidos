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

int cargar_configuracion(const char *config_file, const char *signos_file, void *unused, Config *cfg) {
    (void)unused;
    memset(cfg, 0, sizeof(Config));
    
    FILE *fp = fopen(config_file, "r");
    if (!fp) {
        fprintf(stderr, "Error: No se pudo abrir %s\n", config_file);
        return -1;
    }
    
    cfg->puerto_horoscopo = obtener_valor_int(fp, "puerto_servidor_horoscopo", 5001);
    cfg->tamano_buffer = obtener_valor_int(fp, "tamano_buffer", 1024);
    
    strcpy(cfg->ip_horoscopo, "127.0.0.1");
    
    fclose(fp);
    
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
    
    return 0;
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

void mostrar_configuracion(const Config *cfg) {
    printf("Puerto Horoscopo: %d\n", cfg->puerto_horoscopo);
    printf("Tamano Buffer: %d\n", cfg->tamano_buffer);
    fflush(stdout);
}
