#ifndef CONFIG_H
#define CONFIG_H

#define MAX_SIGNOS 20

typedef struct {
    char ip_horoscopo[64];
    int puerto_horoscopo;
    int tamano_buffer;
    int num_signos;
    char signos[MAX_SIGNOS][50];
    char predicciones[MAX_SIGNOS][256];
} Config;

int cargar_configuracion(const char *config_file, const char *signos_file, void *unused, Config *cfg);
const char* obtener_prediccion(const Config *cfg, const char *signo);
void mostrar_configuracion(const Config *cfg);

#endif
