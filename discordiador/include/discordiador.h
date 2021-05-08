#ifndef DISCORDIADOR_H
#define DISCORDIADOR_H
#include <stdio.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <stdbool.h>
#include "shared_utils.h"
#include "tests.h"


// t_log* iniciar_logger(void);
void terminar_programa(int conexion, t_log* logger, t_config* config);
// t_config* leer_config(void);
void enviar_msj(char* mensaje, int socket_cliente,t_log* logger);

#endif