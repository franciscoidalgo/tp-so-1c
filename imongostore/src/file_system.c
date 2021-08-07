/*
 * file_system.c
 *
 *  Created on: 20 jun. 2021
 *      Author: utnso
 */

#include "fs_functions.h"
#include "../src/file_system.h"
#include "fs_paths.h"
#include "../../shared/include/shared_utils.h"

#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/log.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <pthread.h>
#include <signal.h>
#include <dirent.h>
#include <stdio.h>


//Forward declarations
void iniciar_server_fs();
void atender_cliente();
void realizar_operaciones (void*);

//Variables globales
t_config* config_fs;
t_log* logger_fs;


//Funciones
void incializar_fs(){
	logger_fs = log_create(PATH_LOGGER, "I-Mongo-Store", 1, LOG_LEVEL_INFO);
	config_fs = config_create(PATH_CONFIG);
	iniciar_en_limpio(config_fs, logger_fs);
	signal(SIGUSR1, recuperar_fs);
	iniciar_server_fs();
}

void iniciar_server_fs (){
	int server_fd = iniciar_servidor(logger_fs, config_fs);
	int cliente_fd;
	while(1){
		cliente_fd = esperar_cliente(server_fd, logger_fs);
		atender_cliente (cliente_fd);
	}
}

void atender_cliente (int socket_cliete){
	int cod_op = recibir_operacion(socket_cliete);
	conexion_t* conexion_cliente = malloc(sizeof(conexion_t));
	conexion_cliente->socket_cliente = socket_cliete;
	conexion_cliente->cod_op = cod_op;
	pthread_t conexion;
	pthread_create(&conexion, NULL, (void*) realizar_operaciones, conexion_cliente);
	pthread_detach(conexion);
	
}

void realizar_operaciones(void* conexion){
	int size;
	char msj_bitacora [250];
	conexion_t* conexion_cliente = (conexion_t*) conexion;
	switch (conexion_cliente->cod_op) {
		case MENSAJE: ;
			char* mensaje = (char*) recibir_buffer(&size, conexion_cliente->socket_cliente);
			interpretar_mensaje_discordiador(mensaje);
			break;
		case BITACORA: ;
			char* buffer = recibir_buffer(&size, conexion_cliente->socket_cliente);
			log_info(logger_fs,"mensaje: %s", buffer);
			uint32_t id_patota;
			uint32_t id_tripulante;
			sscanf(buffer, "%lu-%lu-%s", &id_patota, &id_tripulante, msj_bitacora);
			generar_bitacora(id_patota, id_tripulante, msj_bitacora, strlen(msj_bitacora)+1);
			break;
		case SABOTAJE:
			log_info(logger_fs,"Se establecio socket para sabotaje.",conexion_cliente->socket_cliente);
			setear_socket_sabo(conexion_cliente->socket_cliente);
			pthread_exit(NULL);			
			break;
	}
	liberar_conexion(conexion_cliente->socket_cliente);
	pthread_exit(NULL);
}