#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "config.h"
#include "json.h"

static void trim(char *str) {
    char *end;
    while (*str == ' ' || *str == '\t') str++;
    if (*str == 0) return;
    end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) end--;
    *(end + 1) = 0;
}

int cargar_configuracion(const char *config_file, const char *signos_file, Config *cfg) {
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
    
    char *json_str = json_read_file(config_file);
    if (json_str) {
        JsonObject *obj = json_parse_object(json_str);
        if (obj) {
            strncpy(cfg->ip_central, json_get_string(obj, "ip_servidor_central"), 63);
            strncpy(cfg->ip_horoscopo, json_get_string(obj, "ip_servidor_horoscopo"), 63);
            strncpy(cfg->ip_clima, json_get_string(obj, "ip_servidor_clima"), 63);
            cfg->puerto_central = json_get_int(obj, "puerto_servidor_central", 5000);
            cfg->puerto_horoscopo = json_get_int(obj, "puerto_servidor_horoscopo", 5001);
            cfg->puerto_clima = json_get_int(obj, "puerto_servidor_clima", 5002);
            cfg->tamano_cache = json_get_int(obj, "tamano_cache", 100);
            cfg->tamano_buffer = json_get_int(obj, "tamano_buffer", 1024);
            cfg->num_hilos_test = json_get_int(obj, "num_hilos_test", 10);
            cfg->consultas_por_hilo = json_get_int(obj, "consultas_por_hilo", 5);
            json_free_object(obj);
        }
        free(json_str);
    }
    
    if (signos_file) {
        FILE *fp = fopen(signos_file, "r");
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
