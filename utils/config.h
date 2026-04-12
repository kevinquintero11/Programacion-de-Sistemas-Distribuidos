#ifndef CONFIG_H
#define CONFIG_H

#define MAX_SIGNOS 20
#define MAX_PRONOSTICOS 20

typedef struct {
    char ip_central[64];
    char ip_horoscopo[64];
    char ip_clima[64];
    
    int puerto_central;
    int puerto_horoscopo;
    int puerto_clima;
    
    int tamano_cache;
    int tamano_buffer;
    
    int num_hilos_test;
    int consultas_por_hilo;
    
    char signos[MAX_SIGNOS][50];
    char predicciones[MAX_SIGNOS][256];
    int num_signos;
    
    char pronosticos[MAX_PRONOSTICOS][512];
    int num_pronosticos;
} Config;

int cargar_configuracion(const char *config_file, const char *signos_file, const char *pronosticos_file, Config *cfg);
void mostrar_configuracion(const Config *cfg);
const char* obtener_prediccion(const Config *cfg, const char *signo);
const char* obtener_pronostico(const Config *cfg, int index);

#endif
