#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PUERTO_SC 5000
#define BUFFSIZE 1024
#define CACHE_SIZE 100

typedef struct {
    char clave[100];
    char respuesta[BUFFSIZE];
} CacheEntry;

CacheEntry cache[CACHE_SIZE];
int cache_count = 0;

char* buscar_cache(const char* clave) {
    for(int i = 0; i < cache_count; i++) {
        if(strcmp(cache[i].clave, clave) == 0) {
            return cache[i].respuesta;
        }
    }
    return NULL;
}

void guardar_cache(const char* clave, const char* respuesta) {
    if(cache_count < CACHE_SIZE) {
        strncpy(cache[cache_count].clave, clave, sizeof(cache[cache_count].clave)-1);
        strncpy(cache[cache_count].respuesta, respuesta, sizeof(cache[cache_count].respuesta)-1);
        cache_count++;
    }
}

int main(int argc, char *argv[]){

    int sock_fd;
    struct sockaddr_in serv_addr;
    char buffer[BUFFSIZE];
    char signo[50], fecha[50];

    if(argc != 3) {
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
    char clave[100];
    snprintf(clave, sizeof(clave), "%s|%s", signo, fecha);

    char* respuesta_cache = buscar_cache(clave);

    if(respuesta_cache != NULL){
        printf("\n===== RESULTADOS (CACHE) =====\n");
        printf("%s\n", respuesta_cache);
        printf("=============================\n");
        close(sock_fd);
        return 0;
    }

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
    guardar_cache(clave, buffer);

    /* Mostrar resultados al usuario */
    printf("\n===== RESULTADOS =====\n");
    printf("%s\n", buffer);
    printf("======================\n");
    
    /* Cerrar socket y terminar */
    close(sock_fd);
    return 0;

}