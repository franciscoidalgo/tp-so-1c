/*
 * file_system.c
 *
 *  Created on: 20 jun. 2021
 *      Author: utnso
 */

#include "fs_functions.h"
#include "file_system.h"
#include "fs_paths.h"
#include "../../shared/include/shared_utils.h"

#include <commons/config.h>
#include <commons/log.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <pthread.h>
#include <signal.h>
#include <dirent.h>
#include <stdio.h>


//Forward declarations
void server_fs();

//Variables globales
t_config* config_fs;
t_log* logger_fs;


//Funciones
void incializar_fs(){
	logger_fs = log_create(PATH_LOGGER, "I-Mongo-Store", 1, LOG_LEVEL_INFO);
	config_fs = config_create(PATH_CONFIG);
	iniciar_en_limpio(config_fs, logger_fs);
	//pthread_t thread_server;
	//pthread_create(&thread_server, NULL, (void*) server_fs, NULL);
	signal(SIGUSR1, recuperar_fs);
	
	while(1){
		sleep(1000);
	};

}

void server_fs (){
	char* puerto = config_get_string_value(config_fs, "PUERTO");
	crear_conexion("271.0.0.1", puerto);
	while(1){
		sleep(100);
	}
}

