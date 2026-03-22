#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <time.h>
#include "config.h"

#define CONFIG_FILE "config.conf"
#define NUM_PRONOSTICOS 10

const char *pronosticos[] = {
    "Día soleado con cielos despejados. Perfecto para actividades al aire libre.",
    "Nublado con posibilidad de lluvias aisladas en la tarde. Lleva un paraguas.",
    "Temperaturas cálidas, alcanzando los 28°C. Hidrátate adecuadamente.",
    "Vientos fuertes del norte. Precaución si conduces vehículos ligeros.",
    "Tormentas eléctricas prevista para la noche. Mejor quedarse en casa.",
    "Niebla matutina que desaparecerá al mediodía. Visibilidad reducida en horas temprana.",
    "Día parcialmente nublado. Temperaturas agradables entre 18°C y 24°C.",
    "Lluvias persistentes durante todo el día. Evita zonas propensas a inundaciones.",
    "Granizo probable en zonas montañosas. Precaución con cultivos y vehículos.",
    "Aumento de temperatura significativo. Alerta por olas de calor en zonas urbanas."
};

Config cfg;

void *manejar_cliente(void *arg){
    int client_fd = *((int *)arg);
    free(arg);

    char *buffer = malloc(cfg.tamano_buffer);
    char fecha[50];
    int index_pronostico;
    
    if (!buffer) {
        close(client_fd);
        return NULL;
    }

    memset(buffer, 0, cfg.tamano_buffer);

    if (recv(client_fd, buffer, cfg.tamano_buffer - 1, 0) > 0) {
        strncpy(fecha, buffer, sizeof(fecha) - 1);
        
        srand(time(NULL) + strlen(fecha));
        index_pronostico = rand() % NUM_PRONOSTICOS;
        
        snprintf(buffer, cfg.tamano_buffer, "Clima %s: %s", fecha, pronosticos[index_pronostico]);
        send(client_fd, buffer, strlen(buffer), 0);
    }

    free(buffer);
    return NULL;
}


int main(){
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    pthread_t hilo;

    if (cargar_configuracion(CONFIG_FILE, &cfg) < 0) {
        fprintf(stderr, "Error al cargar configuracion. Usando valores por defecto.\n");
    }
    
    mostrar_configuracion(&cfg);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Error al crear socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; 
    server_addr.sin_port = htons(cfg.puerto_clima);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al vincular socket");
        close(server_fd);
        exit(1);
    }

    if (listen(server_fd, 5) < 0) {
        perror("Error al escuchar");
        close(server_fd);
        exit(1);
    }

    printf("Servidor de Clima (SP) escuchando en %s:%d\n", cfg.ip_clima, cfg.puerto_clima);

    while (1) {
        client_len = sizeof(client_addr);
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        
        if (client_fd < 0) {
            perror("Error al aceptar conexión");
            continue;
        }
        
        int *client_fd_ptr = malloc(sizeof(int));
        *client_fd_ptr = client_fd;
        
        if (pthread_create(&hilo, NULL, manejar_cliente, client_fd_ptr) != 0) {
            perror("Error al crear hilo");
            close(client_fd);
            free(client_fd_ptr);
        }
        
        pthread_detach(hilo);
    }
    
    close(server_fd);
    return 0;

}
