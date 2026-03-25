#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/socket.h>
#include <time.h>
#include "config.h"

#define CONFIG_FILE "config.conf"
#define SIGNOS_FILE "signos.txt"

char fechas[][20] = {
    "10/01/2000",
    "11/02/2001",
    "12/03/2002",
    "13/04/2003",
    "14/05/2004"
};

void* cliente(void *arg) {

    Config cfg;
    int id = *((int*)arg);
    free(arg);

    if (cargar_configuracion(CONFIG_FILE, SIGNOS_FILE, NULL, &cfg) < 0) {
        fprintf(stderr, "[Hilo %d] Error al cargar configuracion\n", id);
        return NULL;
    }

    char *buffer = malloc(cfg.tamano_buffer);
    if (!buffer) {
        perror("Error al asignar memoria");
        return NULL;
    }

    for(int i = 0; i < cfg.consultas_por_hilo; i++) {

        int sock_fd;
        struct sockaddr_in serv_addr;

        char signo[50];
        char fecha[50];

        int idx_signo = rand() % cfg.num_signos;
        int idx_fecha = rand() % 5;

        strcpy(signo, cfg.signos[idx_signo]);
        strcpy(fecha, fechas[idx_fecha]);

        sock_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (sock_fd < 0) {
            perror("Error socket");
            return NULL;
        }

        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr(cfg.ip_central);
        serv_addr.sin_port = htons(cfg.puerto_central);

        if(connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
            perror("Error conexion");
            close(sock_fd);
            continue;
        }

        snprintf(buffer, cfg.tamano_buffer, "%s|%s", signo, fecha);

        send(sock_fd, buffer, strlen(buffer), 0);

        memset(buffer, 0, cfg.tamano_buffer);
        recv(sock_fd, buffer, cfg.tamano_buffer - 1, 0);

        printf("\n[HILO %d] Consulta %d\n", id, i+1);
        printf("Signo: %s Fecha: %s\n", signo, fecha);
        printf("Respuesta:\n%s\n", buffer);
        fflush(stdout);

        close(sock_fd);

        usleep(100000);
    }

    free(buffer);
    return NULL;
}

int main() {

    Config cfg;
    pthread_t hilos[100];

    if (cargar_configuracion(CONFIG_FILE, SIGNOS_FILE, NULL, &cfg) < 0) {
        fprintf(stderr, "Error al cargar configuracion\n");
        return 1;
    }

    printf("=== Test de Concurrencia ===\n");
    mostrar_configuracion(&cfg);

    srand(time(NULL));

    for(int i = 0; i < cfg.num_hilos_test; i++) {

        int *id = malloc(sizeof(int));
        *id = i + 1;

        pthread_create(&hilos[i], NULL, cliente, id);
    }

    for(int i = 0; i < cfg.num_hilos_test; i++) {
        pthread_join(hilos[i], NULL);
    }

    printf("\nTEST FINALIZADO\n");
    fflush(stdout);

    return 0;
}
