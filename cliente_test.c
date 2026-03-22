#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/socket.h>
#include <time.h>

#define PUERTO_SC 5000
#define BUFFSIZE 1024
#define NUM_HILOS 10
#define CONSULTAS_POR_HILO 5

char *signos[] = {
    "leo", "aries", "tauro", "cancer", "virgo"
};

char *fechas[] = {
    "10/01/2000",
    "11/02/2001",
    "12/03/2002",
    "13/04/2003",
    "14/05/2004"
};

void* cliente(void *arg) {

    int id = *((int*)arg);
    free(arg);

    for(int i = 0; i < CONSULTAS_POR_HILO; i++) {

        int sock_fd;
        struct sockaddr_in serv_addr;
        char buffer[BUFFSIZE];

        char signo[50];
        char fecha[50];

        /* Selección aleatoria dentro de los arrays */
        int idx = rand() % 5;

        strcpy(signo, signos[idx]);
        strcpy(fecha, fechas[idx]);

        sock_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (sock_fd < 0) {
            perror("Error socket");
            return NULL;
        }

        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        serv_addr.sin_port = htons(PUERTO_SC);

        if(connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
            perror("Error conexion");
            close(sock_fd);
            continue;
        }

        snprintf(buffer, sizeof(buffer), "%s|%s", signo, fecha);

        send(sock_fd, buffer, strlen(buffer), 0);

        memset(buffer, 0, BUFFSIZE);
        recv(sock_fd, buffer, BUFFSIZE - 1, 0);

        printf("\n[HILO %d] Consulta %d\n", id, i+1);
        printf("Signo: %s Fecha: %s\n", signo, fecha);
        printf("Respuesta:\n%s\n", buffer);

        close(sock_fd);

        usleep(200000);
    }

    return NULL;
}

int main() {

    pthread_t hilos[NUM_HILOS];

    srand(time(NULL));  // semilla aleatoria

    for(int i = 0; i < NUM_HILOS; i++) {

        int *id = malloc(sizeof(int));
        *id = i + 1;

        pthread_create(&hilos[i], NULL, cliente, id);
    }

    for(int i = 0; i < NUM_HILOS; i++) {
        pthread_join(hilos[i], NULL);
    }

    printf("\nTEST FINALIZADO\n");

    return 0;
}