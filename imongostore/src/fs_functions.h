#ifndef FS_FUNCTIONS_H
#define FS_FUNCTIONS_H

#include <commons/config.h>
#include <commons/log.h>
#include <inttypes.h>
#include <pthread.h>
#include "../../shared/include/shared_utils.h"


void iniciar_en_limpio(t_config*, t_log*);

void recuperar_fs(int);
void interpretar_mensaje_discordiador (char* mensaje);
void generar_bitacora(char* id_patota, char* tripulante_id, char* entrada, int sizeofentrada);

void setear_socket_sabo(int socket_sabotaje);

#endif