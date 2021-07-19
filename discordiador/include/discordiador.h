#ifndef DISCORDIADOR_H
#define DISCORDIADOR_H
#include <stdio.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <stdbool.h>
#include "shared_utils.h"
#include "tests.h"
#include <pthread.h>


// t_log* iniciar_logger(void);
void terminar_programa(int conexion, t_log* logger, t_config* config);
// t_config* leer_config(void);
void enviar_msj(char* mensaje, int socket_cliente,t_log* logger);

// Enviar
void iniciar_patota(int conexion);
void iniciar_otra_patota(int conexion);
char* enviar_proxima_tarea(int conexion);
char* enviar_actualizar_estado( int conexion);
char* enviar_ubicacion_del_tripulante(int conexion);
char* expulsar_tripulante_1(int conexion);
char* expulsar_tripulante_3(int conexion);
char* expulsar_tripulante_5(int conexion);
char* expulsar_tripulante_1_patota_2(int conexion);
char* expulsar_tripulante_3_patota_2(int conexion);
char* expulsar_tripulante_5_patota_2(int conexion);

char* envio_compactar(int conexion);



#endif