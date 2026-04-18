#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
    char ip_central[64];
    char ip_horoscopo[64];
    char ip_clima[64];
    int puerto_central;
    int puerto_horoscopo;
    int puerto_clima;
    int tamano_cache;
    int tamano_buffer;
} Config;

int cargar_configuracion(const char *config_file, void *unused, Config *cfg);
void mostrar_configuracion(const Config *cfg);

#endif
