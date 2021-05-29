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

// Memoria
void* reservar_espacio_de_memoria();
// void* reservar_espacio_de_memoria(t_config*);

// Creaciones de Estructuras administrativas
void crear_patota(int pid,t_list* lista);
t_list* crear_tareas(t_list* lista);

// Recibir
uint32_t recibir_catidad_de_tripulantes(t_list* lista);



// Archivos
void leer_archivo(char* path);


// Finalizacion de programa
void terminar_miramhq(t_log* logger, t_config* config);






// Testeos
void testear_biblioteca_compartida();
void testear_enviar_mensaje(int conexion);
void validar_malloc();

#endif