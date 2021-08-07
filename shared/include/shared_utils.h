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
#include<commons/string.h>
#include<commons/config.h>
#include<string.h>

char* mi_funcion_compartida();

//#define IP "127.0.0.1"
//#define PUERTO "4444"

// TADs - estructuras
typedef enum	//un tipo de forma para discriminar los diferentes tipos de mensajes que se peuden enviar
{
	MENSAJE,
	PAQUETE,
	INICIAR_PATOTA,
	RECIBIR_LA_UBICACION_DEL_TRIPULANTE,
	ENVIAR_PROXIMA_TAREA,
	ACTUALIZAR_ESTADO,
	EXPULSAR_TRIPULANTE,
	COMPACTAR,
	DUMP,
	INICIAR_TRIPULANTE,
	FINALIZACION,
	SABOTAJE,
	SABOTAJE_RESUELTO,
	BITACORA
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

// Structs de Mi RAM

// La seniora patota (PCB - proceso)
typedef struct	// Tamanio de 8 Bytes
{
	uint32_t pid;		// Id de la patota
	uint32_t tareas;	// Indica el COMIENZO de la lista
}__attribute__((packed))
t_pcb;


// El senior tripulante (TCB - hilo)
typedef struct	// Tamanio de 21 Bytes
{
	uint32_t tid;			// Id del tripulante
	char estado;			// Estado del tripulante (New/Ready/Exec/Blocked)
	uint32_t posicion_x;	// Pos x
	uint32_t posicion_y;	// Pos y
	uint32_t proxima_instruccion;	// instruccion que el tripulante debera hacer
	uint32_t puntero_pcb;	// quien es mi patota?
}__attribute__((packed))
t_tcb;

// fin de Structs de Patota

// Structs I-Mongo

typedef struct
{
	uint32_t id_tripulante;
	uint32_t id_patota;
	uint32_t length_mensaje;
	char* mensaje;
}__attribute__((packed))
t_bitacora;

// funciones bitacora
void enviar_bitacora(t_bitacora* bitacora, int socket_cliente);
char* obtener_bitacoras(uint32_t id_patota, uint32_t id_tripulante);

// inciar servidor
int iniciar_servidor(t_log* logger,t_config* config);

// creacion
	// administracion
t_config* leer_config(char* name);
t_log* iniciar_logger(char* name);
int crear_conexion(char* ip, char* puerto);

	// estructura
t_paquete* crear_paquete(int);
t_paquete* crear_super_paquete(void);

// validacion
void validar_logger(t_log* logger);
void validar_config(t_config* config);

// recibir
void* recibir_buffer(int*, int);
void recibir_paquete(int, t_list*);
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