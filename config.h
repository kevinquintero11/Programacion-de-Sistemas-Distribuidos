#ifndef CONFIG_H
#define CONFIG_H

#define MAX_LINEA 256
#define MAX_SIGNOS 20
#define MAX_PREDICCIONES 20

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
    int num_signos;
    
    char predicciones[MAX_SIGNOS][256];
    int num_predicciones;
} Config;

int cargar_configuracion(const char *archivo, Config *cfg);
void mostrar_configuracion(const Config *cfg);
const char* obtener_prediccion(const Config *cfg, const char *signo);

#endif
