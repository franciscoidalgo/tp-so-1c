#ifndef DISCORDIADOR_H
#define DISCORDIADOR_H
#include <stdio.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <stdbool.h>
#include "shared_utils.h"
#include<pthread.h>
#include<commons/string.h>
#include<commons/collections/dictionary.h>
#include<semaphore.h>
#include <readline/readline.h>
#include <readline/history.h>


typedef struct t_tarea{	// Tamanio de 16 Bytes+strlen(tarea). Aunque en memoria debe ser todo char*
	char* accion;			// Accion de la tarea
	int parametro;		// Numero relacionado a la tarea
	uint32_t posicion_x;	// Pos x
	uint32_t posicion_y;	// Pos y
	uint32_t tiempo;		// Tiempo en realizar la tarea
}__attribute__ ((packed)) t_tarea;

// typedef struct t_comunicacion{
//     int socket_MIRAM;
//     int socket_IMONGOSTORE; 
// }__attribute__ ((packed)) t_comunicacion;

// El senior tripulante (TCB - hilo)
typedef struct t_tripulante   // Tamanio de 21 Bytes
{   
    int tid;        // Id del tripulante
    int tid_imongostore;
    uint32_t posicion_x;    // Pos x
    uint32_t posicion_y;    // Pos y
    t_tarea* tarea;    // instruccion que el tripulante debera hacer
    uint32_t puntero_pcb;    // quien es mi patota?
    char estado;        // Estado del tripulante (New/Ready/Exec/Blocked)
    int QUANTUM_ACTUAL;
    // int socket_MIRAM;
    // int socket_IMONGOSTORE; 
}__attribute__ ((packed)) t_tripulante;

t_list* BLOCKED;
t_list* EXIT;
t_list* READY;
t_list* BLOCKED_EMERGENCY;
t_list* EXEC;
t_list* NEW;

t_log* logger;
t_log* config;
t_dictionary * dic_datos_consola;

int CONEXION_MIRAM;
int CONEXION_IMONGOSTORE; 

int ID_PATOTA;

enum input_consola{
INICIAR_PATOTA_,
INICIAR_PLANIFICACION,
PAUSAR_PLANIFICACION,
EXPULSAR_TRIPULANTE_,
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

enum algoritmo{
FIFO,
RR,
};


//INICIALIZACION DE SEMAFOROS
pthread_mutex_t mutex_mostrar_por_consola = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_lista_new = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_lista_ready = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_lista_blocked = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_lista_exit = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_lista_exec = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_entrada_salida = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_planificacion = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_sabotaje = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_pausar =PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t condicion_pausear_planificacion;
pthread_cond_t semaforo_sabotaje;
pthread_barrier_t barrera;
sem_t sem_IO;
sem_t sem_IO_queue;
sem_t sem_exe;
sem_t sem_exe_notificacion;
//INICIALIZACION DE SEMAFOROS

bool EXISTE_SABOTAJE;
bool PAUSEAR_PLANIFICACION;

//variables globales de archivo de configuracion
char* IP_MI_RAM_HQ;
char* PUERTO_MI_RAM_HQ;
char* IP_I_MONGO_STORE;
char* PUERTO_I_MONGO_STORE;
int GRADO_MULTITAREA;
int ALGORITMO;
int QUANTUM;
int DURACION_SABOTAJE;
int RETARDO_CICLO_CPU;
//variables globales de archivo de configuracion

void enviar_mensaje_and_codigo_op(char* mensaje,int codop ,int socket_cliente);
void perder_tiempo(int* i);
void iterator(t_tripulante* t);
t_paquete* armar_paquete(char* palabra);
t_tripulante* crear_tripulante(uint32_t patota, uint32_t posx, uint32_t posy, uint32_t id);
void buscar_tarea_a_RAM(t_tripulante* tripu);
void inicializar_variables();
void terminar_variables_globales(int socket);
void enviar_tareas_a_RAM(int conexion,char** argv);
void recepcionar_patota(char** argv); //agregar tripulantes a lista NEW
void iterator_lines_free(char* string);
int get_diccionario_accion(char* accion);
bool es_tripu_de_patota(int patota, int id_tripu,t_tripulante* tripu);
void* recibir_mensaje_de_RAM(int socket_cliente, t_log* logger,int* direccion_size);
t_tarea* deserealizar_tarea(t_buffer* buffer);
t_tarea* recibir_tarea_de_RAM(int socket);
void planificar_FIFO(t_tripulante *tripulante);
void entrada_salida();
void realizar_tarea_exe(t_tripulante* tripulante);
void atender_accion_de_consola(char* retorno_consola);
void add_queue(int lista, t_tripulante* tripulante);
void atender_sabotaje();
void mover_a_la_posicion_de_la_tarea(t_tripulante* tripu);
void iniciar_planificacion();
void expulsar_si_no_hay_tarea(t_tripulante * tripu);
void realizar_tarea_comun(t_tripulante * tripulante);
bool es_tarea_comun(t_tripulante* tripulante);
void peticion_ES(t_tripulante* tripulante);
void planificar_RR(t_tripulante *tripulante);
void realizar_tarea_comun_RR(t_tripulante *tripulante);
void moverme_hacia_tarea_RR(t_tripulante *tripulante);
void verificar_existencia_de_sabotaje();
void planificar_FIFO_con_sabotaje();
void activar_sabotaje();
void desactivar_sabotaje();
void moverme_hacia_tarea_en_sabotaje(t_tripulante *tripulante,int x, int y);
void sacar_tripulantes_de_BLOCKED_EMERGENCY();
void resolver_sabotaje_por_tripulante_mas_cercano_a_posicion(int x,int y);
void agregar_tripulantes_a_BLOCKED_EMERGENCY_en_sabotaje();
void verificar_existencia_de_pausado();
void pausar_planificacion();
void continuar_planificacion();
int obtener_algoritmo(char* algoritmo_de_planificacion);
void enviar_nuevo_estado_a_ram(t_tripulante* tripulante);
void enviar_expulsar_tripulante_a_ram(t_tripulante *tripulante);
void enviar_posicion_a_ram(t_tripulante* tripulante,int socket);
void enviar_info_para_bitacora_a_imongostore(t_tripulante *tripulante,int socket);
void buscar_proxima_a_RAM_o_realizar_peticion_de_entrada_salida(t_tripulante *tripulante);
void mover_tripulante_entre_listas_si_existe(int lista_origen,int lista_destino,int patota, int id_tripu);
void control_de_tripulantes_listos(t_tripulante* tripu);
t_list *obtener_lista(int lista);
void controlar_forma_de_salida(t_tripulante* t);
void loggear_linea();
void enviar_movimiento_a_imongo_store_para_BITACORA(t_tripulante *tripulante);
void enviar_comienzo_o_finalizacion_de_tarea_a_imongo_store_para_BITACORA(t_tripulante *tripulante,char* msj);
void enviar_mensajes_en_sabotaje_a_imongo_store_para_BITACORA(t_tripulante *tripulante,char* mensaje);
void volver_a_actividad();

#endif