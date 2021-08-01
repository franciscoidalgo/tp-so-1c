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

// Funciones_hilos
// Paginacion
void iniciar_patota_1_PAG(int conexion);
void iniciar_patota_2_PAG(int conexion);
void iniciar_patota_3_PAG(int conexion);
char* expulsar_tripulante_1_patota_1_PAG(int conexion);
char* expulsar_tripulante_1_patota_3_PAG(int conexion);

// Segmentacion
void iniciar_patota_A_SEG(int conexion);
void iniciar_patota_B_SEG(int conexion);
void iniciar_patota_C_SEG(int conexion);
char* expulsar_tripulante_1_patota_A_SEG(int conexion);
char* expulsar_tripulante_2_patota_A_SEG(int conexion);
char* expulsar_tripulante_3_patota_A_SEG(int conexion);
char* expulsar_tripulante_4_patota_A_SEG(int conexion);
char* expulsar_tripulante_1_patota_B_SEG(int conexion);
char* expulsar_tripulante_2_patota_B_SEG(int conexion);
char* expulsar_tripulante_3_patota_B_SEG(int conexion);
char* expulsar_tripulante_4_patota_B_SEG(int conexion);
char* expulsar_tripulante_1_patota_C_SEG(int conexion);
char* enviar_ubicacion_del_tripulante_1_de_la_patota_A_1er_movimiento(int conexion);
char* enviar_ubicacion_del_tripulante_1_de_la_patota_A_2do_movimiento(int conexion);
char* enviar_ubicacion_del_tripulante_1_de_la_patota_A_3ro_movimiento(int conexion);
char* enviar_ubicacion_del_tripulante_1_de_la_patota_A_4rto_movimiento(int conexion);


// Enviar
void iniciar_patota(int conexion);

void iniciar_otra_patota(int conexion);
void iniciar_tercera_patota(int conexion);
char* enviar_proxima_tarea(int conexion);
char* enviar_actualizar_estado( int conexion);
char* enviar_ubicacion_del_tripulante(int conexion);
char* expulsar_tripulante_1(int conexion);
char* expulsar_tripulante_2(int conexion);
char* expulsar_tripulante_3(int conexion);
char* expulsar_tripulante_5(int conexion);
char* expulsar_tripulante_1_patota_2(int conexion);
char* expulsar_tripulante_3_patota_2(int conexion);
char* expulsar_tripulante_5_patota_2(int conexion);

char* envio_compactar(int conexion);
char* envio_dump(int conexion);



#endif