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
pthread_mutex_t cache_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    char clave[100];
    char respuesta[4096];
} CacheEntry;

CacheEntry *cache;
int cache_count = 0;
int cache_size;
Config cfg;

char* buscar_cache(const char* clave) {
	pthread_mutex_lock(&cache_mutex);
    printf("clave recibida: %s\n", clave);
    for(int i = 0; i < cache_count; i++) {
        printf("clave %d: %s\n", i, cache[i].clave);
        if(strcmp(cache[i].clave, clave) == 0) {
            pthread_mutex_unlock(&cache_mutex);
            return cache[i].respuesta;
        }
    }
	pthread_mutex_unlock(&cache_mutex);
    return NULL;
}

void guardar_cache(const char* clave, const char* respuesta) {
	pthread_mutex_lock(&cache_mutex);
    if(cache_count < cache_size) {
        strncpy(cache[cache_count].clave, clave, sizeof(cache[cache_count].clave)-1);
        strncpy(cache[cache_count].respuesta, respuesta, sizeof(cache[cache_count].respuesta)-1);
        cache_count++;
    }
	pthread_mutex_unlock(&cache_mutex);
}

int conectar_y_consultar(const char *ip, int puerto, const char *dato, char *respuesta) {
	int sockfd;
	struct sockaddr_in server_addr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		return -1;

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip);
	server_addr.sin_port = htons(puerto);

	if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		close(sockfd);
		return -1;
	}

	if (send(sockfd, dato, strlen(dato), 0) < 0) {
		close(sockfd);
		return -1;
	}

	memset(respuesta, 0, cfg.tamano_buffer);
	recv(sockfd, respuesta, cfg.tamano_buffer - 1, 0);

	close(sockfd);
	return 0;
}

void* manejar_cliente(void *arg){
    int client_fd = *((int *)arg);
    free(arg);

    char *buffer = malloc(cfg.tamano_buffer);
    char *horoscopo = malloc(cfg.tamano_buffer);
    char *clima = malloc(cfg.tamano_buffer);
    char *resultado_final = malloc(cfg.tamano_buffer * 2);
    char signo[50], fecha[50];
    
    if (!buffer || !horoscopo || !clima || !resultado_final) {
        free(buffer); free(horoscopo); free(clima); free(resultado_final);
        close(client_fd);
        return NULL;
    }

    memset(buffer, 0, cfg.tamano_buffer);
	if (recv(client_fd, buffer, cfg.tamano_buffer - 1, 0) <= 0) {
		free(buffer);
		free(horoscopo);
		free(clima);
		free(resultado_final);
		close(client_fd);
		return NULL;
	}

	char clave[100];
	strncpy(clave, buffer, sizeof(clave) - 1);
	
	char* respuesta_cache = buscar_cache(clave);

    if(respuesta_cache != NULL){
        printf("\n===== RESULTADOS (CACHE) =====\n");
        printf("%s\n", respuesta_cache);
        printf("=============================\n");
		send(client_fd, respuesta_cache, strlen(respuesta_cache), 0);
        close(client_fd);
        return NULL;
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

    printf("Consultando al Servidor de Horóscopo...\n");
    if (conectar_y_consultar(cfg.ip_horoscopo, cfg.puerto_horoscopo, signo, horoscopo) < 0) {
        strcpy(horoscopo, "Error al consultar horóscopo");
    }

    printf("Consultando al Servidor de Clima...\n");
    if (conectar_y_consultar(cfg.ip_clima, cfg.puerto_clima, fecha, clima) < 0){
        strcpy(clima, "Error al consultar clima");
    }

    snprintf(resultado_final, cfg.tamano_buffer * 2, "%s\n%s", horoscopo, clima);
	
	printf("respuesta SC: %s\n", resultado_final);

	send(client_fd, resultado_final, strlen(resultado_final), 0);

	guardar_cache(clave, resultado_final);

	close(client_fd);
	return NULL;
}


int main(){
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    pthread_t hilo;

    if (cargar_configuracion(CONFIG_FILE, NULL, &cfg) < 0) {
        fprintf(stderr, "Error al cargar configuracion. Usando valores por defecto.\n");
    }
    
    cache_size = cfg.tamano_cache;
    cache = calloc(cache_size, sizeof(CacheEntry));
    if (!cache) {
        fprintf(stderr, "Error al asignar memoria para cache\n");
        exit(1);
    }

    mostrar_configuracion(&cfg);

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0){
		perror("Error al crear socket");
		exit(1);
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(cfg.puerto_central);

	if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		perror("Error al vincular socket");
		close(server_fd);
		exit(1);
	}

	if (listen(server_fd, 5) < 0){
		perror("Error al escuchar");
		close(server_fd);
		exit(1);
	}

    printf("Servidor Central (SC) escuchando en puerto %d\n", cfg.puerto_central);
	printf("Conectando a SH (%s:%d) y SP (%s:%d)\n", 
           cfg.ip_horoscopo, cfg.puerto_horoscopo,
           cfg.ip_clima, cfg.puerto_clima);

	while (1) {
		client_len = sizeof(client_addr);
		client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);

		if (client_fd < 0) {
			perror("Error al aceptar conexión");
			continue;
		}

		int *client_fd_ptr = malloc(sizeof(int));
		*client_fd_ptr = client_fd;

		if (pthread_create(&hilo, NULL, manejar_cliente, client_fd_ptr) != 0){
			perror("Error al crear hilo");
			close(client_fd);
			free(client_fd_ptr);
		}

		pthread_detach(hilo);
	}

	free(cache);
	close(server_fd);
	return 0;

}
