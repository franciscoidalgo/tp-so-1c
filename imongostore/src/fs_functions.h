#ifndef FS_FUNCTIONS_H
#define FS_FUNCTIONS_H

#include <commons/config.h>
#include <commons/log.h>
#include <inttypes.h>

void iniciar_en_limpio(t_config*, t_log*);

void recuperar_fs(int);
void interpretar_mensaje_discordiador (char* mensaje);
void generar_bitacora(uint32_t tripulante_id, char* entrada, int sizeofentrada);

#endif