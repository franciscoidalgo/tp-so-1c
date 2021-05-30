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


// El senior tripulante (TCB - hilo)
typedef struct    // Tamanio de 21 Bytes
{
    uint32_t tid;        // Id del tripulante
    char estado;        // Estado del tripulante (New/Ready/Exec/Blocked)
    uint32_t posicion_x;    // Pos x
    uint32_t posicion_y;    // Pos y
    uint32_t proxima_instruccion;    // instruccion que el tripulante debera hacer
    uint32_t puntero_pcb;    // quien es mi patota?
}__attribute__ ((packed)) t_tcb;

t_list* BLOCKED;
t_list* EXIT;
t_list* READY;
t_list* NEW;

struct barrera_s {
        int n;
        pthread_mutex_t mutex;
        pthread_cond_t llegaron;
} barrera;


t_log* logger;

t_log* iniciar_logger(void);
void terminar_programa(int conexion, t_log* logger, t_config* config);
t_config* leer_config(void);
void enviar_msj(char* mensaje, int socket_cliente);
void perder_tiempo(int* i);
void iterator(t_tcb* t);
t_paquete* armar_paquete(char* palabra);
t_tcb* crear_tripulante(uint32_t patota, uint32_t posx, uint32_t posy, uint32_t id);
void buscar_tarea_a_RAM(void* tripu);

#endif