#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "config.h"

#define CONFIG_FILE "config.conf"
#define SIGNOS_FILE "signos.txt"

int main(int argc, char *argv[]){

    Config cfg;
    int sock_fd;
    struct sockaddr_in serv_addr;
    char *buffer;
    char signo[50], fecha[50];

    if (cargar_configuracion(CONFIG_FILE, SIGNOS_FILE, NULL, &cfg) < 0) {
        fprintf(stderr, "Error al cargar configuracion\n");
        exit(1);
    }

    buffer = malloc(cfg.tamano_buffer);
    if (!buffer) {
        perror("Error al asignar memoria");
        exit(1);
    }

    if(argc != 3) {
        fprintf(stderr, "Entrada esperada: %s <signo> <dd/mm/yy>\n", argv[0] );
        fprintf(stderr, "Ejemplo: %s leo <12/08/1999>\n", argv[0] );
        exit(1);
    }

    strncpy(signo, argv[1], sizeof(signo) - 1);
    strncpy(fecha, argv[2], sizeof(fecha) - 1);

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("Error al crear socket");
        exit(1);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr  = inet_addr(cfg.ip_central);
    serv_addr.sin_port = htons(cfg.puerto_central);

    if(connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        perror("Error al intentar conectarse con el servidor central");
    }

     printf("Conectado al Servidor Central (%s:%d)\n", cfg.ip_central, cfg.puerto_central);
    
    char clave[100];
    snprintf(clave, sizeof(clave), "%s|%s", signo, fecha);

    strncpy(buffer, clave, sizeof(clave) - 1);

    if (send(sock_fd, buffer, strlen(buffer), 0) < 0) {
        perror("Error al enviar datos");
        exit(1);
    }

     printf("Consulta enviada: Signo=%s, Fecha=%s\n", signo, fecha);

    memset(buffer, 0, cfg.tamano_buffer);
    if (recv(sock_fd, buffer, cfg.tamano_buffer - 1, 0) < 0) {
        perror("Error al recibir respuesta");
        exit(1);
    }

    printf("\n===== RESULTADOS =====\n");
    printf("%s\n", buffer);
    printf("======================\n");
    
    free(buffer);
    close(sock_fd);
    return 0;

}
