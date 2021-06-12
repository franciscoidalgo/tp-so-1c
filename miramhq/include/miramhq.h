#ifndef MIRAMHQ_H
#define MIRAMHQ_H
#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <stdbool.h>
#include "shared_utils.h"
#include "tests.h"
#include <malloc.h>
#include <pthread.h>
#include <string.h>

#define TAMANIO_PCB 8
#define TAMANIO_TCB 21

void* MEMORIA;

// Structs - de administracion

typedef struct
{
    uint32_t inicio;
    uint32_t fin;

}__attribute__((packed))
t_segmento_libre;


typedef struct
{
    uint32_t bytes_libres;
    t_list* segmentos_libres;
}__attribute__((packed))
administrador_de_segmentacion;


// Orden de listas
bool orden_lista_admin_segmentacion(t_segmento_libre* segmento_libre_A, t_segmento_libre* segmento_libreB);

// administrador_de_segmentacion* admin_segmentacion;

// Memoria
void reservar_espacio_de_memoria();
void liberar_espacio_de_memoria();

// Segmentacion
void iniciar_segmentacion();
void asignar_segmento(uint32_t);
t_segmento_libre* buscar_segmento_libre(uint32_t bytes_ocupados);
void liberar_segmento(t_segmento_libre* segmento);


// void* reservar_espacio_de_memoria(t_config*);

// Creaciones de Estructuras administrativas
void crear_patota(int pid,t_list* lista);
t_list* crear_tareas(t_list* lista);

// Recibir
uint32_t recibir_catidad_de_tripulantes(t_list* lista);



// Finalizacion de programa
void terminar_miramhq(t_log* logger, t_config* config);






// Testeos
void testear_biblioteca_compartida();
void testear_enviar_mensaje(int conexion);
void validar_malloc();

void testear_asignar_y_liberar_segmentacion();


// Archivos
void leer_archivo(char* path);


#endif