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

Config cfg;

void *manejar_cliente(void *arg){
    int client_fd = *((int *)arg);
    free(arg);

    char *buffer = malloc(cfg.tamano_buffer);
    char signo[50];
    
    if (!buffer) {
        close(client_fd);
        return NULL;
    }

    memset(buffer, 0, cfg.tamano_buffer);

    if (recv(client_fd, buffer, cfg.tamano_buffer - 1, 0) > 0) {
        strncpy(signo, buffer, sizeof(signo) - 1);
        
        const char *prediccion = obtener_prediccion(&cfg, signo);
        
        snprintf(buffer, cfg.tamano_buffer, "Horoscopo %s: %s", signo, prediccion);
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
    server_addr.sin_port = htons(cfg.puerto_horoscopo);

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

    printf("Servidor de Horoscopo (SH) escuchando en %s:%d\n", cfg.ip_horoscopo, cfg.puerto_horoscopo);

    while (1) {
        client_len = sizeof(client_addr);
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        
        if (client_fd < 0) {
            perror("Error al aceptar conexion");
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
