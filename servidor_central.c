#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <time.h>
#define PUERTO_SC 5000 /* Puerto del Servidor Central */
#define PUERTO_SH 5001 /* Puerto del Servidor de Horóscopo */
#define PUERTO_SP 5002 /* Puerto del Servidor de Clima */
#define BUFFSIZE 1024	 /* Tamaño del buffer */
#define CACHE_SIZE 100
pthread_mutex_t cache_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    char clave[100];
    char respuesta[BUFFSIZE];
} CacheEntry;

CacheEntry cache[CACHE_SIZE];
int cache_count = 0;



char* buscar_cache(const char* clave) {
	pthread_mutex_lock(&cache_mutex);
    printf("clave recibida: %s\n", clave);
    for(int i = 0; i < cache_count; i++) {
        printf("clave %d: %s\n", i, cache[i].clave);
        if(strcmp(cache[i].clave, clave) == 0) {
            return cache[i].respuesta;
        }
    }
	pthread_mutex_unlock(&cache_mutex);
    return NULL;
}

void guardar_cache(const char* clave, const char* respuesta) {
	pthread_mutex_lock(&cache_mutex);
    if(cache_count < CACHE_SIZE) {
        strncpy(cache[cache_count].clave, clave, sizeof(cache[cache_count].clave)-1);
        strncpy(cache[cache_count].respuesta, respuesta, sizeof(cache[cache_count].respuesta)-1);
        cache_count++;
    }
	pthread_mutex_unlock(&cache_mutex);
}


int conectar_y_consultar(const char *ip, int puerto, const char *dato, char *respuesta) {
	int sockfd;
	struct sockaddr_in server_addr;

	/* Crear socket */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		return -1;

	/* Configurar dirección del servidor */
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip);
	server_addr.sin_port = htons(puerto);

	/* Conectar al servidor */
	if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		close(sockfd);
		return -1;
	}

	/* Enviar consulta */
	if (send(sockfd, dato, strlen(dato), 0) < 0) {
		close(sockfd);
		return -1;
	}

	/* Recibir respuesta */
	memset(respuesta, 0, BUFFSIZE);
	recv(sockfd, respuesta, BUFFSIZE - 1, 0);

	close(sockfd);
	return 0;
}

void* manejar_cliente(void *arg){
    int client_fd = *((int *)arg);
    free(arg);

    char buffer[BUFFSIZE];
    char signo[50], fecha[50];
	char horoscopo[BUFFSIZE], clima[BUFFSIZE];
    char resultado_final[BUFFSIZE * 2];

    /* Recibir consulta del cliente */
	memset(buffer, 0, BUFFSIZE);
	if (recv(client_fd, buffer, BUFFSIZE - 1, 0) <= 0) {
		close(client_fd);
		return NULL;
	}

	char clave[100];
	char claveOriginal[100];
	strncpy(clave, buffer, sizeof(clave) - 1);
	
	char* respuesta_cache = buscar_cache(clave);

    if(respuesta_cache != NULL){
        printf("\n===== RESULTADOS (CACHE) =====\n");
        printf("%s\n", respuesta_cache);
        printf("=============================\n");
		send(client_fd, respuesta_cache, strlen(respuesta_cache), 0);
        close(client_fd);
        return 0;
    }

    char *token = strtok(buffer, "|");
	if (token){
        strncpy(signo, token, sizeof(signo) - 1);
    }
		
	token = strtok(NULL, "|");
	if (token){
        strncpy(fecha, token, sizeof(fecha) - 1);
    }

	printf("Consulta recibida: Signo=%s, Fecha=%s\n", signo, fecha);

    /* Consultar al Servidor de Horóscopo */
    printf("Consultando al Servidor de Horóscopo...\n");
    if (conectar_y_consultar("127.0.0.1", PUERTO_SH, signo, horoscopo) < 0) {
        strcpy(horoscopo, "Error al consultar horóscopo");
    }

    /* Consultar al Servidor de Clima */
    printf("Consultando al Servidor de Clima...\n");
    if (conectar_y_consultar("127.0.0.1", PUERTO_SP, fecha, clima) < 0){
        strcpy(clima, "Error al consultar clima");
    }

    /* Combinar resultados */
    snprintf(resultado_final, sizeof(resultado_final), "%s\n%s", horoscopo, clima);
	
	printf("respuesta SC: %s\n", resultado_final);

    /* Enviar resultado al cliente */
	send(client_fd, resultado_final, strlen(resultado_final), 0);

	guardar_cache(clave, resultado_final);

    /* Cerrar conexión */
	close(client_fd);
	return NULL;
}

int main(){
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    pthread_t hilo;

    /* Crear socket TCP */
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0){
		perror("Error al crear socket");
		exit(1);
	}

    /* Configurar dirección del servidor */
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(PUERTO_SC);

    /* Vincular socket al puerto */
	if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		perror("Error al vincular socket");
		close(server_fd);
		exit(1);
	}

    /* Escuchar conexiones */
	if (listen(server_fd, 5) < 0){
		perror("Error al escuchar");
		close(server_fd);
		exit(1);
	}

    printf("Servidor Central (SC) escuchando en puerto %d\n", PUERTO_SC);
	printf("Conectando a SH (puerto %d) y SP (puerto %d)\n", PUERTO_SH, PUERTO_SP);

    /* Loop principal: aceptar clientes y crear hilos */
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

		if (pthread_create(&hilo, NULL, manejar_cliente, client_fd_ptr) != 0){
			perror("Error al crear hilo");
			close(client_fd);
			free(client_fd_ptr);
		}

		pthread_detach(hilo);
	}

	close(server_fd);
	return 0;

}