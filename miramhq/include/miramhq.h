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

#include <nivel-gui/nivel-gui.h>
#include <nivel-gui/tad_nivel.h>
#include <ncurses.h>

#define TAMANIO_PCB 8
#define TAMANIO_TCB 21

void* MEMORIA;
void *MEMORIA_VIRTUAL;
uint32_t* ESTADO_MARCOS;
uint32_t* ESTADO_MARCOS_VIRTUALES;
uint32_t* TIMESTAMP_MARCOS;
uint32_t COUNTER_LRU = 1;
uint32_t PUNTERO_CLOCK = 0;
uint32_t* ARRAY_BIT_USO;
uint32_t MODO_DESALOJO; //0 LRU 1 CLOCK
t_list* TABLA_DE_PAGINAS;
char* dump_segmentacion;
int TAMANIO_PAGINAS;
int TAMANIO_MEMORIA;
int TAMANIO_MEMORIA_VIRTUAL;
int CANTIDAD_MARCOS;
int CANTIDAD_MARCOS_VIRTUALES;
int OFFSET = 15000;
char ID_MAPA = 'A';
t_list* TABLA_DE_MAPA;

#define VariableName(name) #name

// Mapa
typedef struct{
    uint32_t pid;			
    uint32_t tid;
    uint32_t pos_x;
    uint32_t pos_y;
    char mapid;
} t_mapa;

// Structs - de administracion

typedef struct
{
    uint32_t inicio;
    uint32_t fin;
    uint32_t se_encuentra;
    char tipo;  //t / p / i   en teoria no hace falta por el protocolo de ordenamiento que cree

}
t_segmento;


typedef struct
{
    uint32_t bytes_libres;
    t_list* segmentos_libres;
}__attribute__((packed))
administrador_de_segmentacion;


typedef struct
{
    t_list* segmentos;           // cada uno ocupa 21 bytes - la cant son los 1ros 4 bytes del pcb
}
t_tabla_de_segmentos;

typedef struct {
    		uint32_t pid;
			t_list* lista_de_tids;
    		t_list* lista_de_marcos;
			t_list* lista_de_presencia;
		} t_tabla_proceso;

typedef struct{
			t_pcb pcb;			
			t_list* lista_de_tcb; 		
			char* lista_de_tareas;
		} estructura_administrativa_paginacion;


typedef struct{
			int tamanio_data;			
			void* data_empaquetada; 		
		} dto_memoria;

typedef struct{
			uint32_t marco;			
			char* estado;
			uint32_t proceso;
			uint32_t pagina; 		
		} dump_memoria;

typedef struct{
			uint32_t pid;			
			uint32_t indice;			
			uint32_t inicio;			
			uint32_t tamanio;			
		} dump_memoria_segmentacion;



// Orden de listas
bool orden_mayor_a_menor(int entero_A, int entero_B);
bool orden_lista_admin_segmentacion(t_segmento* segmento_libre_A, t_segmento* segmento_libreB);
bool orden_lista_admin_segmentacion_best_free(t_segmento* segmento_libre_A, t_segmento* segmento_libreB);
bool orden_lista_admin_segmentacion_best_free_mayor_a_menor(t_segmento* segmento_libre_A, t_segmento* segmento_libre_B);

// administrador_de_segmentacion* admin_segmentacion;

/* -------------------------MEMORIA----------------------------- */
// Mapa
void iniciar_mapa();
void eliminar_mapa();
void crear_personaje(char id, uint32_t pos_x, uint32_t pos_y);
void mostrar_patotas_presentes_en_mapa();
void mover_personaje(char id, uint32_t pos_x, uint32_t pos_y);
void expulsar_tripulante_en_mapa(uint32_t pid, uint32_t tid);
void mover_personaje_en_mapa(uint32_t pid,uint32_t tid, char id,uint32_t pos_x_nuevo, uint32_t pos_y_nuevo);
void eliminar_personaje(char id);
void eliminar_personaje_ubicado(char id, uint32_t pos_x, uint32_t pos_y);
void refrescar_tabla_de_mapas();
void iniciar_patota_en_mapa(uint32_t pid, t_list* lista_tcb);
char obtener_id_mapa(uint32_t pid, uint32_t tid);



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
bool es_necesario_compactar(int tamanio_tareas, int cantidad_de_tripulantes);
void guardar_en_MEMORIA_tcb(t_segmento* segmento_a_ocupar,t_tcb* tcb);
void guardar_en_MEMORIA_tareas(t_segmento* segmento_a_ocupar,char* tareas_unidas);

// Segmentacion
void iniciar_segmentacion();
void asignar_segmento(uint32_t);
t_segmento* buscar_segmento_libre(uint32_t bytes_ocupados);
void liberar_segmento(t_segmento* segmento);
void iniciar_patota_SEGMENTACION (t_list* lista,t_pcb* pcb,t_list* tcbs, char* tareas_unidas, int cant_tripulantes);
char* enviar_proxima_tarea_SEGMENTACION (uint32_t pid, uint32_t tid);
void actualizar_estado_SEGMENTACION (uint32_t pid_actualizar_estado, uint32_t tid_actualizar_estado, char estado_actualizar_estado);
void expulsar_tripulante_SEGMENTACION(uint32_t pid_expulsar_tripulante,uint32_t tid_expulsar_tripulante);
void cambiar_ubicacion_tripulante_SEGMENTACION(uint32_t pid_ubicacion, uint32_t tid_ubicacion, uint32_t nueva_pos_x, uint32_t nueva_pos_y);
void hacer_dump_SEGMENTACION();



// Paginacion
void iniciar_paginacion();
uint32_t obtener_tamanio_array_de_marcos ();
uint32_t obtener_tamanio_array_de_marcos_virtuales ();
dto_memoria empaquetar_data_paginacion (estructura_administrativa_paginacion* data_a_empaquetar);
void modificar_tlb (uint32_t id_proceso, t_list* lista_tids, t_list* marcos_utilizados);
uint32_t obtener_marcos_vacios();
uint32_t obtener_marcos_vacios_virtuales();
uint32_t paginas_que_ocupa(uint32_t cantidad_bytes);
void setear_memoria(t_list* lista_a_reservar, void* data_empaquetada);
void pasar_un_marco_de_memoria(uint32_t marco_virtual, uint32_t marco_principal, bool sentido);
t_list* obtener_marcos_a_reservar(uint32_t paginas_solicitadas);
void setear_marco_como_usado(uint32_t numero_marco);
void setear_marcos_usados(t_list* lista_a_reservar);
uint32_t indice_del_minimo_de_un_array(uint32_t* array, uint32_t tamanio_array);
uint32_t obtener_proceso_de_marco(uint32_t marco);
void reemplazar_marco_de_tabla_de_paginas(uint32_t marco_a_reemplazar, uint32_t nuevo_marco, uint32_t presencia);
uint32_t obtener_marco_virtual_vacio();
void desalojar_un_marco(uint32_t marco_a_desalojar);
t_list* obtener_marcos_a_desalojar(uint32_t numero_de_paginas_a_desalojar);
void desalojar(uint32_t numero_de_paginas_a_desalojar);
t_list* listar_tids(estructura_administrativa_paginacion* dato);
void iniciar_patota_PAGINACION (estructura_administrativa_paginacion* dato_a_paginar);
t_tabla_proceso* buscar_proceso(uint32_t id_proceso);
uint32_t indice_de_tripulante(t_list* lista_de_ids, uint32_t id_tripulante);
t_list* lista_marcos_en_virtual(t_list* lista_de_paginas, uint32_t id_proceso);
void setear_nueva_posicion(t_list* marcos_a_cambiar, uint32_t posx, uint32_t posy, uint32_t desplazamiento);
void cambiar_ubicacion_tripulante_PAGINACION(uint32_t id_proceso, uint32_t id_tripulante, uint32_t nueva_posx, uint32_t nueva_posy);
uint32_t redondear_para_arriba (uint32_t numero, uint32_t divisor);
t_list* obtener_marcos_de_paginas(t_list* lista_de_marcos, uint32_t pagina_inicio, uint32_t pagina_fin);
void liberar_marcos(t_list* lista_marcos_borrado);
void liberar_tabla(t_list* marcos, t_list* presencia, uint32_t pagina_inicio, uint32_t pagina_fin);
uint32_t modificar_direccion_tareas(t_list* marcos_a_modificar, uint32_t dezplazamiento);
void necesito_en_ppal(uint32_t direccion_logica_inicio, uint32_t direccion_logica_fin, uint32_t id_proceso);
void reemplazar_marco_de_tabla_por_indice(uint32_t id_proceso, uint32_t nuevo_marco, uint32_t numero_pagina, uint32_t presencia);
uint32_t obtener_indice_de_marco(uint32_t marco, uint32_t id_proceso);
void alojar(t_list* lista_marcos_en_virtual);
t_list* obtener_marcos_segun_direccion_logica(uint32_t direccion_logica_inicio, uint32_t direccion_logica_fin, t_list* lista_de_marcos_completa);
uint32_t obtener_indice_del_proceso(uint32_t pid);
uint32_t tamanio_lista_tareas(uint32_t id_proceso, uint32_t direccion_logica_tareas);
t_list* compactar_tripulante(uint32_t id_proceso, uint32_t direccion_logica_inicio, uint32_t ultima_direccion_logica, uint32_t lista_tareas);
void expulsar_tripulante_PAGINACION(uint32_t id_proceso, uint32_t id_tripulante);
t_list* deconstruir_string(char* array);
uint32_t obtener_direccion_logica_tareas(uint32_t id_proceso);
char* obtener_lista_de_tareas(uint32_t direccion_logica, uint32_t id_proceso);
uint32_t obtener_id_proxima_tarea(uint32_t id_proceso, uint32_t id_tripulante);
char* proxima_tarea_PAGINACION(uint32_t id_proceso, uint32_t id_tripulante);
void actualizar_estado_PAGINACION(uint32_t id_proceso, uint32_t id_tripulante, char nuevo_estado);
char* estado_marco(uint32_t marco);
void hacer_dump_PAGINACION();
void barrer_un_proceso(uint32_t marco);
void llenar_lista(t_tabla_proceso* item_tabla);
void mostrar_array_marcos();
void mostrar_array_marcos_virtuales();
void mostrar_lista(t_list* lista);
void mostrar_tabla_de_paginas();



// Tarea
char* removeDigits(char* input);
char* unir_tareas(t_list* lista);
int tamanio_de_tareas(t_list* lista);
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
void liberar_patota_si_no_hay_tripulantes(int pid);
void colocar_ultimo_segmento_libre_al_final();
t_list* lista_de_segmentos_ocupados();





/* -----------------------Fin MEMORIA--------------------------- */

/* -----------------------Iteradores---------------------------- */

void iterator_agregar_tareas(char* value);
void iterator_destroy(char* value);
void iterator_destroy_tcb(t_tcb* tcb);
void iterator_lines_free(char* string);
void iterator_segmentos_free(t_segmento* segmento);
void iterator_posicion(char* pos);
void iterator_patotas_presentes(t_list* patota);
void iterator_pcb(t_pcb* pcb);
void iterator_tcb(t_tcb* tcb);
void iterator_segmento(t_segmento* segmento_libre);
bool orden_lista_admin_segmentacion(t_segmento* segmento_libre_A, t_segmento* segmento_libreB);
bool orden_lista_pcbs(t_pcb* pcb_a, t_pcb* pcb_b);
bool orden_lista_mapa(t_mapa* mapa_a, t_mapa* mapa_b);
bool orden_lista_segmentos(t_segmento* seg_a, t_segmento* seg_b);
bool orden_lista_segmentos_contraria(t_segmento* seg_a, t_segmento* seg_b);
bool comparador_patotas(t_list* seg_patota_a, t_list* seg_patota_b);
bool final_maximo(t_segmento* seg_a, t_segmento* seg_b);
bool condicion_segmento_afectado(t_segmento* seg);
bool condicion_segmento_presente_en_memoria(t_segmento* seg);
bool condicion_patota_presente_en_memoria(t_list* segmentos_patota);
void transformacion_segmento_afectado(t_segmento* seg);
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
void imprimir_patotas_presentes_dump();
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
char* trimwhitespace(char *str);


#endif