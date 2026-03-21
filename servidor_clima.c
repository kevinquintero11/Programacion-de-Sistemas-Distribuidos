#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define PUERTO_SH 5002
#define BUFFSIZE 1024
#define NUM_PRONOSTICOS 10

/* Array con predicciones aleatorias del horóscopo */
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

void *manejar_cliente(void *arg){
    int client_fd = *((int *)arg);
    free(arg);

    char buffer[BUFFSIZE];
    char fecha[50];
    int index_pronostico;

    memset(buffer, 0, BUFFSIZE);

    /* Leer los datos que envia el cliente */
    if (recv(client_fd, buffer, BUFFSIZE - 1, 0) > 0) {
        strncpy(fecha, buffer, sizeof(fecha) - 1);
        
        /* Seleccionar predicción aleatoria usando el fecha como semilla */
        srand(time(NULL) + strlen(fecha));
        index_pronostico = rand() % NUM_PRONOSTICOS;
        
        /* Enviar predicción al cliente */
        snprintf(buffer, BUFFSIZE, "Clima %s: %s", fecha, pronosticos[index_pronostico]);
        send(client_fd, buffer, strlen(buffer), 0);
    }
}


int main(){
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    pthread_t hilo;

    /* Crear socket TCP */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Error al crear socket");
        exit(1);
    }

    /* Configurar direccion del servidor*/
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; 
    server_addr.sin_port = htons(PUERTO_SH);

    /* Vincular socket al puerto */
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al vincular socket");
        close(server_fd);
        exit(1);
    }

    /* Escucha conexiones (hasta 5 en cola) */
    if (listen(server_fd, 5) < 0) {
        perror("Error al escuchar");
        close(server_fd);
        exit(1);
    }

    printf("Servidor de Clima (SP) escuchando en puerto %d\n", PUERTO_SH);

    while (1) {
        client_len = sizeof(client_addr);
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        
        if (client_fd < 0) {
            perror("Error al aceptar conexión");
            continue;
        }
        
        /* Crear hilo para atender al cliente */
        int *client_fd_ptr = malloc(sizeof(int));
        *client_fd_ptr = client_fd;
        
        if (pthread_create(&hilo, NULL, manejar_cliente, client_fd_ptr) != 0) {
            perror("Error al crear hilo");
            close(client_fd);
            free(client_fd_ptr);
        }
        
        /* Desvinclar hilo para que no espere su terminacion */
        pthread_detach(hilo);
    }
    
    close(server_fd);
    return 0;

}