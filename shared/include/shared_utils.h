#ifndef SHARED_UTILS_H
#define SHARED_UTILS_H

#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <commons/config.h>
#include <string.h>

char* mi_funcion_compartida();

//#define IP "127.0.0.1"
//#define PUERTO "4444"

// TADs - estructuras
typedef enum	//un tipo de forma para discriminar los diferentes tipos de mensajes que se peuden enviar
{
	MENSAJE,
	PAQUETE
}op_code;

//t_log* logger;

typedef struct
{
	int size;		//Tamanio del payload
	void* stream;	//Payload
} t_buffer;

typedef struct
{
	op_code codigo_operacion;	//para saber que tipo de TAD es enviado/recibido
	t_buffer* buffer;			//Tiene lo que nos interesa y su tamanio
} t_paquete;

// inciar servidor
int iniciar_servidor(t_log* logger);

// creacion
	// administracion
t_config* leer_config(char* name);
t_log* iniciar_logger(char* name);
int crear_conexion(char* ip, char* puerto);

	// estructura
t_paquete* crear_paquete(void);
t_paquete* crear_super_paquete(void);

// validacion
void validar_logger(t_log* logger);
void validar_config(t_config* config);

// recibir
void* recibir_buffer(int*, int);
t_list* recibir_paquete(int);
void recibir_mensaje(int socket_cliente, t_log* logger,int* direccion_size);
int recibir_operacion(int);

// enviar
void enviar_mensaje(char* mensaje, int socket_cliente);
void enviar_paquete(t_paquete* paquete, int socket_cliente);

// agregar
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);

// liberacion - Eliminacion
void liberar_conexion(int socket_cliente);
void eliminar_paquete(t_paquete* paquete);
void terminar_programa(int conexion, t_log* logger, t_config* config);

// espera
int esperar_cliente(int socket_servidor,t_log* logger);


#endif