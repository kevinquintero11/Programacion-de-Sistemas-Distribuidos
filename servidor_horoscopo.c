#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define PUERTO_SH 5001
#define BUFFSIZE 1024
#define NUM_PREDICCIONES 10

/* Array con predicciones aleatorias del horóscopo */
const char *predicciones[] = {
    "Hoy será un día excepcional para ti. Las estrellas alinean tu camino hacia el éxito.",
    "Un encuentro inesperado cambiará tu perspectiva sobre la vida. Mantén los ojos abiertos.",
    "Tu energía está en su punto máximo. Es el momento perfecto para tomar decisiones importantes.",
    "La paciencia te llevará a donde quieres. No te apresures, el universo trabaja a tu favor.",
    "Nuevas oportunidades aparecerán. Aprende a reconocerlas y aprovéchalas.",
    "Tu creatividad estará inspirada. Expresa tus ideas, serán bien recibidas.",
    "Un amigo necesitará tu ayuda. Bríndala sin esperar nada a cambio.",
    "Las finanzas mejoran significativamente. Invierte con sabiduría.",
    "Tu estado emocional está equilibrado. Disfruta de la paz interior.",
    "El amor llama a tu puerta. No temas abrirla y déjate llevar."
};

void *manejar_cliente(void *arg){
    int client_fd = *((int *)arg);
    free(arg);

    char buffer[BUFFSIZE];
    char signo[50];
    int index_prediccion;

    memset(buffer, 0, BUFFSIZE);

    /* Leer los datos que envia el cliente */
    if (recv(client_fd, buffer, BUFFSIZE - 1, 0) > 0) {
        strncpy(signo, buffer, sizeof(signo) - 1);
        
        /* Seleccionar predicción aleatoria usando el signo como semilla */
        srand(time(NULL) + strlen(signo));
        index_prediccion = rand() % NUM_PREDICCIONES;
        
        /* Enviar predicción al cliente */
        snprintf(buffer, BUFFSIZE, "Horóscopo %s: %s", signo, predicciones[index_prediccion]);
        send(client_fd, buffer, strlen(buffer), 0);
    }

    return NULL;
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

    printf("Servidor de Horóscopo (SH) escuchando en puerto %d\n", PUERTO_SH);

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