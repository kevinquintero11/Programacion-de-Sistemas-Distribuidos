#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
    char ip_central[64];
    int puerto_central;
    int tamano_buffer;
} Config;

int cargar_configuracion(const char *config_file, Config *cfg);

#endif
