#ifndef DISCORDIADOR_H
#define DISCORDIADOR_H
#include <stdio.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <stdbool.h>
#include "shared_utils.h"
#include "tests.h"
#include<pthread.h>
#include<commons/string.h>
#include<commons/collections/dictionary.h>
#include<semaphore.h>


typedef struct	// Tamanio de 16 Bytes+strlen(tarea). Aunque en memoria debe ser todo char*
{   
	char* accion;			// Accion de la tarea
	uint32_t parametro;		// Numero relacionado a la tarea
	uint32_t posicion_x;	// Pos x
	uint32_t posicion_y;	// Pos y
	uint32_t tiempo;		// Tiempo en realizar la tarea
}t_tarea;//__attribute__((packed))

// El senior tripulante (TCB - hilo)
typedef struct t_tcb   // Tamanio de 21 Bytes
{
    uint32_t tid;        // Id del tripulante
    uint32_t posicion_x;    // Pos x
    uint32_t posicion_y;    // Pos y
    t_tarea* tarea;    // instruccion que el tripulante debera hacer
    uint32_t puntero_pcb;    // quien es mi patota?
    char estado;        // Estado del tripulante (New/Ready/Exec/Blocked)
    int QUANTUM_ACTUAL;
}__attribute__ ((packed)) t_tcb;

int NIVEL_MULTIPROCESAMIENTO;

t_list* BLOCKED;
t_list* EXIT;
t_list* READY;
t_list* BLOCKED_EMERGENCY;
t_list* EXEC;
t_list* NEW;

t_log* logger;
t_log* config;
t_dictionary * dic_datos_consola;

char* IP_RAM;
char* PUERTO_RAM;
char* IP_IMONGO;
char* PUERTO_IMONGO;

int ID_PATOTA;

enum input_consola{
INICIAR_PATOTA,
INICIAR_PLANIFICACION,
PAUSAR_PLANIFICACION,
EXPULSAR_TRIPULANTE,
LISTAR_TRIPULANTE,
OBTENER_BITACORA,
};

enum queue_enum{
_NEW_,
_READY_,
_BLOCKED_,
_BLOCKED_EMERGENCY_,
_EXIT_,
_EXEC_,
};


//INICIALIZACION DE SEMAFOROS
pthread_mutex_t mutex =PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_mostrar_por_consola = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_lista_new = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_lista_ready = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_lista_blocked = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_lista_exit = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_lista_exec = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_planificacion = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t iniciarPlanificacion;
pthread_barrier_t barrera;
sem_t sem_IO;
sem_t sem_IO_queue;
sem_t sem_exe;

bool primerIntento;

void enviar_mensaje_and_codigo_op(char* mensaje,int codop ,int socket_cliente);
void perder_tiempo(int* i);
void iterator(t_tcb* t);
t_paquete* armar_paquete(char* palabra);
t_tcb* crear_tripulante(uint32_t patota, uint32_t posx, uint32_t posy, uint32_t id);
void buscar_tarea_a_RAM(t_tcb* tripu);
void inicializar_variables();
void terminar_variables_globales(int socket);
void enviar_tareas_a_RAM(int conexion,char** argv);
void recepcionar_patota(char** argv); //agregar tripulantes a lista NEW
void iterator_lines_free(char* string);
int get_diccionario_accion(char* accion);
bool es_tripu_de_id(int id,t_tcb* tripulante);
void expulsar_tripu(t_list* lista, int id_tripu);
t_tcb* remover_tripu(t_list* lista, int id_tripu);
void* recibir_mensaje_de_RAM(int socket_cliente, t_log* logger,int* direccion_size);
t_tarea* deserealizar_tarea(t_buffer* buffer);
t_tarea* recibir_tarea_de_RAM(int socket);
void planificar_FIFO(t_tcb *tripulante);
void entrada_salida();
void realizar_tarea_exe(t_tcb* tripulante);
void atender_accion_de_consola(char* retorno_consola);
void add_queue(int lista, t_tcb* tripulante);
void atender_sabotaje();
void moverme_hacia_tarea(t_tcb* tripu);
void iniciar_planificacion();
void consultar_proxima_tarea(t_tcb * tripu);
void realizar_tarea_comun(t_tcb * tripulante);
bool es_tarea_comun(t_tcb* tripulante);
void peticion_ES(t_tcb* tripulante);
void planificar_RR(t_tcb *tripulante);
void realizar_tarea_comun_RR(t_tcb *tripulante);
void moverme_hacia_tarea_RR(t_tcb *tripulante);

#endif