#ifndef CONFIG_H
#define CONFIG_H

#define MAX_SIGNOS 20

typedef struct {
    char ip_central[64];
    int puerto_central;
    int tamano_buffer;
    int num_hilos_test;
    int consultas_por_hilo;
    int num_signos;
    char signos[MAX_SIGNOS][50];
} Config;

int cargar_configuracion(const char *config_file, Config *cfg);
void mostrar_configuracion(const Config *cfg);

#endif
