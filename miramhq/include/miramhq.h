#ifndef MIRAMHQ_H
#define MIRAMHQ_H
#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/memory.h>
#include <stdbool.h>
#include "shared_utils.h"
#include "tests.h"
#include <malloc.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>

#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TAMANIO_PCB 8
#define TAMANIO_TCB 21

void* MEMORIA;
#define VariableName(name) #name

// Structs - de administracion

typedef struct
{
    uint32_t inicio;
    uint32_t fin;
    uint32_t se_encuentra;
    char tipo;  //t / p / i   en teoria no hace falta por el protocolo de ordenamiento que cree

}__attribute__((packed))
t_segmento;


typedef struct
{
    uint32_t bytes_libres;
    t_list* segmentos_libres;
}__attribute__((packed))
administrador_de_segmentacion;

typedef enum	//un tipo de forma para discriminar los diferentes tipos de mensajes que se peuden enviar
{
	FIRSTFIT,
	BESTFIT
}criterio_code;


// typedef struct
// {
//     uint32_t pcb_byte_inicial;          // este ocupa siempre 8 bytes
//     t_list* tcb_byte_inicial;           // cada uno ocupa 21 bytes - la cant son los 1ros 4 bytes del pcb
//     t_list* tcbs_en_memoria_bool;       // en principio no se necesitaria pero para no ir a buscar a memoria se jusdtificaria
//     uint32_t tareas_byte_inicial;       // Hay que calcular el tamanio, se necesita guardarlo
//     int tareas_bytes_ocupados;
// }__attribute__((packed))
// t_tabla_de_segmentos;

typedef struct
{
    t_list* segmentos;           // cada uno ocupa 21 bytes - la cant son los 1ros 4 bytes del pcb
}
t_tabla_de_segmentos;

// typedef struct
// {
//     t_segmento* pcb;          // este ocupa siempre 8 bytes
//     t_list* tcbs;           // cada uno ocupa 21 bytes - la cant son los 1ros 4 bytes del pcb
//     t_segmento* tareas;       // Hay que calcular el tamanio, se necesita guardarlo
// }__attribute__((packed))
// t_tabla_de_segmentos;


// 

// lista de segmentos por patota
// lista de lo de arriba

// Orden de listas
bool orden_lista_admin_segmentacion(t_segmento* segmento_libre_A, t_segmento* segmento_libreB);
bool orden_lista_admin_segmentacion_best_free(t_segmento* segmento_libre_A, t_segmento* segmento_libreB);

// administrador_de_segmentacion* admin_segmentacion;

/* -------------------------MEMORIA----------------------------- */
// Memoria
void mostrar_memoria_char();
void mostrar_memoria_entero();
void limpiar_memoria();
void reservar_espacio_de_memoria();
void liberar_espacio_de_memoria();
t_segmento* ultimo_segmento_libre();
t_segmento* ultimo_segmento_libre_compactable();
void consolidar_segmentos_libres_se_es_posbile();
t_list* segmentos_ocupados_afectados(t_list* lista_ocupados);
int tareas_bytes_ocupados(t_list* tareas);
// int cantidad_de_tareas(char* tareas);
t_segmento* buscar_segmento_libre_first_fit(uint32_t bytes_ocupados);
t_segmento* buscar_segmento_libre_best_fit(uint32_t bytes_ocupados);
void guardar_en_MEMORIA_tcb(t_segmento* segmento_a_ocupar,t_tcb* tcb);
void guardar_en_MEMORIA_tareas(t_segmento* segmento_a_ocupar,char* tareas_unidas);

// Segmentacion
void iniciar_segmentacion();
void asignar_segmento(uint32_t);
t_segmento* buscar_segmento_libre(uint32_t bytes_ocupados);
void liberar_segmento(t_segmento* segmento);

// Tarea
void guardar_tarea_segmentacion(t_tarea* tarea);
char* removeDigits(char* input);
char* unir_tareas(t_list* lista);
char* retornar_tareas(t_segmento* segmento_tareas);
int cantidad_de_tareas (char* tareas);
int cantidad_de_apariciones (char* string, char caracter);
char* retornar_tarea_solicitada(char* tareas_en_MEMORIA,int nro_de_tarea);


// Tcb
int retornar_tid_del_tcb(t_segmento* segmento_tcb);
char retornar_estado_del_tcb(t_segmento* segmento_tcb);
int retornar_pos_x_del_tcb(t_segmento* segmento_tcb);
int retornar_pos_y_del_tcb(t_segmento* segmento_tcb);
int retornar_prox_inst_del_tcb(t_segmento* segmento_tcb);
uint32_t retornar_puntero_pid_del_tcb(t_segmento* segmento_tcb);
t_list* retornar_segmentos_patota(uint32_t pid);
t_segmento* retornar_segmento_tcb(t_list* segmentos_patota,int tid);
t_segmento* retornar_segmento_tareas(t_list* segmentos_patota);
t_segmento* retornar_segmento_pcb(t_list* segmentos_patota);
void actualizar_en_MEMORIA_tcb_prox_tarea(t_segmento* segmento_a_modificar);
void actualizar_en_MEMORIA_tcb_estado(t_segmento* segmento_a_modificar, char estado);
void actualizar_en_MEMORIA_tcb_posiciones(t_segmento* segmento_a_modificar, uint32_t pos_x, uint32_t pos_y);
t_tcb* retornar_tcb(t_segmento* segmento_tcb);



// Pcb
t_pcb* retornar_pcb(t_segmento* segmento_pcb);
uint32_t retornar_pid_del_pcb(t_segmento* segmento_pcb);
uint32_t retornar_inicio_de_tareas_del_pcb(t_segmento* segmento_pcb);

// Compactacion
void compactar();
void desplazar_memoria(int nro, int diferencia);
void colocar_ultimo_segmento_libre_al_final();
t_list* lista_de_segmentos_ocupados();





/* -----------------------Fin MEMORIA--------------------------- */

/* -----------------------Iteradores---------------------------- */

void iterator_agregar_tareas(char* value);
void iterator_destroy(char* value);
void iterator_destroy_tcb(t_tcb* tcb);
void iterator_destroy_tarea(t_tarea* tarea);
void iterator_lines_free(char* string);
void iterator_segmentos_free(t_segmento* segmento);
void iterator_posicion(char* pos);
void iterator_patotas_presentes(t_list* patota);
void iterator_pcb(t_pcb* pcb);
void iterator_tcb(t_tcb* tcb);
void iterator_tarea(t_tarea* tarea);
void iterator_segmento(t_segmento* segmento_libre);
bool orden_lista_admin_segmentacion(t_segmento* segmento_libre_A, t_segmento* segmento_libreB);
bool orden_lista_pcbs(t_pcb* pcb_a, t_pcb* pcb_b);
bool orden_lista_segmentos(t_segmento* seg_a, t_segmento* seg_b);
bool comparador_patotas(t_list* seg_patota_a, t_list* seg_patota_b);
bool final_maximo(t_segmento* seg_a, t_segmento* seg_b);
bool condicion_segmento_afectado(t_segmento* seg);
bool condicion_segmento_presente_en_memoria(t_segmento* seg);
void transformacion_segmento_afectado(t_segmento* seg);
void iterator_tarea_cargar_a_memoria(t_tarea* tarea);
void iterator(char* value);
void iterator_patota(t_list* segmentos_patota);


/* ---------------------Fin Iteradores-------------------------- */



// void* reservar_espacio_de_memoria(t_config*);
void atender_cliente(int cliente_fd);
void atender_cliente_SEGMENTACION(int cliente_fd);
void atender_cliente_PAGINACION(int cliente_fd);



// Creaciones de Estructuras administrativas
void crear_patota(int pid,t_list* lista);
t_list* crear_tareas(t_list* lista);
t_list* crear_tcbs(t_list* tripulantes_ubicados,int cant_tripulantes);

// Recibir
uint32_t recibir_pid(t_list* lista);
uint32_t recibir_catidad_de_tripulantes(t_list* lista);
t_list* recibir_posiciones_de_los_tripulantes(t_list* lista);
char recibir_estado(t_list* lista);




// Finalizacion de programa
void terminar_miramhq(t_log* logger, t_config* config);

// Impresiones
void loggear_entero(int entero);
void loggear_entero_en_texto(char* entero);
void loggear_entero_con_texto(char* texto,int entero);
void loggear_info_segmentos(t_segmento* segmento, char* nombre_de_segmento);
void loggear_info_segmento(t_segmento* segmento, char* nombre_de_segmento);
void imprimir_patotas_presentes();
void mostrar_fecha();
void loggear_linea();
void loggear_tcb (t_segmento* segmento_tcb);


// Testeos
void testear_biblioteca_compartida();
void testear_enviar_mensaje(int conexion);
void validar_malloc();

void testear_asignar_y_liberar_segmentacion();

// Signals
void my_signal_kill(int sig);
void my_signal_compactar(int sig);


// Envios
void enviar_proxima_tarea(char* tarea_solicitada, int cliente_fd);


// Archivos
void leer_archivo(char* path);


#endif