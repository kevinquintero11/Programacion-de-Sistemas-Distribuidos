#ifndef CONFIG_H
#define CONFIG_H

#define MAX_PRONOSTICOS 20

typedef struct {
    char ip_clima[64];
    int puerto_clima;
    int tamano_buffer;
    int num_pronosticos;
    char pronosticos[MAX_PRONOSTICOS][512];
} Config;

int cargar_configuracion(const char *config_file, const char *pronosticos_file, Config *cfg);
const char* obtener_pronostico(const Config *cfg, int index);
void mostrar_configuracion(const Config *cfg);

#endif
