#ifndef DISCORDIADOR_H
#define DISCORDIADOR_H
#include <stdio.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/queue.h>
#include <stdbool.h>
#include "shared_utils.h"
#include "tests.h"
#include<pthread.h>
#include<commons/string.h>
#include<commons/collections/dictionary.h>
#include<sys/sem.h>

typedef struct	// Tamanio de 16 Bytes+strlen(tarea). Aunque en memoria debe ser todo char*
{
	char* accion;			// Accion de la tarea
	uint32_t parametro;		// Numero relacionado a la tarea
	uint32_t posicion_x;	// Pos x
	uint32_t posicion_y;	// Pos y
	uint32_t tiempo;		// Tiempo en realizar la tarea
}__attribute__((packed))
t_tarea;

// El senior tripulante (TCB - hilo)
typedef struct    // Tamanio de 21 Bytes
{
    uint32_t tid;        // Id del tripulante
    uint32_t posicion_x;    // Pos x
    uint32_t posicion_y;    // Pos y
    t_tarea* tarea;    // instruccion que el tripulante debera hacer
    uint32_t puntero_pcb;    // quien es mi patota?
    char estado;        // Estado del tripulante (New/Ready/Exec/Blocked)
}__attribute__ ((packed)) t_tcb;


t_list* BLOCKED;
t_list* EXIT;
t_list* READY;
t_list* NEW;

t_log* logger;
t_log* config;
t_dictionary * dic_datos_consola;
char* IP;
char* PUERTO;

enum input_consola{
INICIAR_PATOTA,
INICIAR_PLANIFICACION,
PAUSAR_PLANIFICACION,
EXPULSAR_TRIPULANTE,
LISTAR_TRIPULNATE,
OBTENER_BITACORA,
};

pthread_mutex_t mutexSalirDeNEW = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_Lista_accciones = PTHREAD_MUTEX_INITIALIZER;
int semaforo;
pthread_t hilo_consola;


void enviar_msj(char* mensaje, int socket_cliente);
void perder_tiempo(int* i);
void iterator(t_tcb* t);
t_paquete* armar_paquete(char* palabra);
t_tcb* crear_tripulante(uint32_t patota, uint32_t posx, uint32_t posy, uint32_t id);
void buscar_tarea_a_RAM(void* tripu);
void inicializar_variables();
void terminar_variables_globales(int socket);
void enviar_tareas_a_RAM(int conexion,char** argv);
void recepcionar_patota(char** argv); //agregar tripulantes a lista NEW
void busqueda_de_tareas_por_patota();
void iterator_lines_free(char* string);
#endif