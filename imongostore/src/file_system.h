/*
 * file_system.h
 *
 *  Created on: 20 jun. 2021
 *      Author: utnso
 */

#ifndef FILE_SYSTEM_H_
#define FILE_SYSTEM_H_

#include <commons/log.h>
#include "fs_paths.h"

typedef struct{
    int socket_cliente;
    int cod_op;
}conexion_t;

void incializar_fs();

#endif /* FILE_SYSTEM_H_ */
