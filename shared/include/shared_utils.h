#ifndef SHARED_UTILS_H
#define SHARED_UTILS_H

#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/collections/list.h>
#include<string.h>

char* mi_funcion_compartida();

//#define IP "127.0.0.1"
//#define PUERTO "4444"

typedef enum
{
	MENSAJE,
	PAQUETE
}op_code;

//t_log* logger;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

void* recibir_buffer(int*, int);

int iniciar_servidor(t_log* logger);
int esperar_cliente(int socket_servidor,t_log* logger);
t_list* recibir_paquete(int);
void recibir_mensaje(int socket_cliente, t_log* logger);
int recibir_operacion(int);

int crear_conexion(char* ip, char* puerto);
void enviar_mensaje(char* mensaje, int socket_cliente);
t_paquete* crear_paquete(void);
t_paquete* crear_super_paquete(void);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void liberar_conexion(int socket_cliente);
void eliminar_paquete(t_paquete* paquete);

#endif