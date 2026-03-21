#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PUERTO_SC 5000
#define BUFFSIZE 1024

int main(int argc, char *argv[]){

    int sock_fd;
    struct sockaddr_in serv_addr;
    char buffer[BUFFSIZE];
    char signo[50], fecha[50];

    if(argc != 3){ // si no recibe 3 argumentos (programa, signo, fecha)
        fprintf(stderr, "Entrada esperada: %s <signo> <dd/mm/yy>\n", argv[0] );
        fprintf(stderr, "Ejemplo: %s leo <12/08/1999>\n", argv[0] );
        exit(1);
    }

    strncpy(signo, argv[1], sizeof(signo) - 1);
    strncpy(fecha, argv[2], sizeof(fecha) - 1);

    /* Crear socket TCP */
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("Error al crear socket");
        exit(1);
    }

    /* Configurar dirección del servidor */
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; // direccion tipo ipv4
    serv_addr.sin_addr.s_addr  = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(PUERTO_SC) ;

    // Conexion con el servidor central
    if(connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        perror("Error al intentar conectarse con el servidor central");
    }

     printf("Conectado al Servidor Central\n");
    
    /* Preparar mensaje: signo|fecha */
    snprintf(buffer, BUFFSIZE, "%s|%s", signo, fecha);

    /* Enviar consulta al SC */
    if (send(sock_fd, buffer, strlen(buffer), 0) < 0) {
        perror("Error al enviar datos");
        exit(1);
    }

     printf("Consulta enviada: Signo=%s, Fecha=%s\n", signo, fecha);

    /* Recibir respuesta del SC */
    memset(buffer, 0, BUFFSIZE);
    if (recv(sock_fd, buffer, BUFFSIZE - 1, 0) < 0) {
        perror("Error al recibir respuesta");
        exit(1);
    }

    /* Mostrar resultados al usuario */
    printf("\n===== RESULTADOS =====\n");
    printf("%s\n", buffer);
    printf("======================\n");
    
    /* Cerrar socket y terminar */
    close(sock_fd);
    return 0;

}