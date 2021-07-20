#ifndef IMONGOSTORE_H
#define IMONGOSTORE_H
#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include "shared_utils.h"
#include "tests.h"
#include <commons/string.h>
#include<pthread.h>


typedef struct	// Tamanio de 16 Bytes+strlen(tarea). Aunque en memoria debe ser todo char*
{
    uint32_t accion_length;
	char* accion;			// Accion de la tarea
	uint32_t parametro;		// Numero relacionado a la tarea
	uint32_t posicion_x;	// Pos x
	uint32_t posicion_y;	// Pos y
	uint32_t tiempo;		// Tiempo en realizar la tarea
}__attribute__((packed))
t_tarea;

typedef struct{
    uint32_t pid;
    t_list* tareas;
}t_tareas;


int CONTADOR;

t_tareas* TAREAS_GLOBAL;

t_list* lista_tareas;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void enviar_tarea(t_tarea* tarea,int unSocket);
void atender_cliente(int socket_cliente);
void iterator(char* value);

t_log* logger;
t_list* lista;

#endif