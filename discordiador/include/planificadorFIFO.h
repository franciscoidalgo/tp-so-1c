#include "discordiador.h"

void planificar_FIFO(t_tripulante *tripulante)
{
	//buscar tarea inicial a RAM
	buscar_tarea_a_RAM(tripulante);
	add_queue(_READY_, tripulante);

	while (1)
	{
		//para pausar la planificacion, en accion de PAUSAR_PLANIFICACION se decrementara un semaforo que estará en funciones de exe y entrada_salida
		control_de_tripulantes_listos(tripulante);
		sem_wait(&sem_exe); //semaforo de multiprocesamiento
		if (tripulante->estado == 'F')
		{
			sem_post(&sem_exe);
			break;
		}
		tripulante = list_remove(READY, 0);
		add_queue(_EXEC_, tripulante);

		do
		{
			mover_a_la_posicion_de_la_tarea(tripulante);
			realizar_tarea_comun(tripulante);
			buscar_proxima_a_RAM_o_realizar_peticion_de_entrada_salida(tripulante);
			expulsar_si_no_hay_tarea(tripulante);
		} while (tripulante->estado == 'E');
	}
}

void control_de_tripulantes_listos(t_tripulante *tripu)
{
	if (list_size(READY) == 0 && list_size(BLOCKED) == 0)
		pthread_exit((void *)pthread_self());
	if (list_size(READY) == 0 && list_size(BLOCKED) == 1)
		sem_wait(&sem_exe_notificacion);
}

void entrada_salida()
{
	// while (1)
	// {
		// sem_wait(&sem_IO_queue);
		pthread_mutex_lock(&mutex_entrada_salida);

		log_info(logger,"%s","Entre nadie mas tiene que entrar aca porque hay pina");
		// sem_wait(&sem_IO); //semaforo de entrada salida

		t_tripulante *tripulante_io = list_get(BLOCKED, 0);

		log_info(logger, "EJECUTANDO IO soy tripu %d de patota %d", tripulante_io->tid, tripulante_io->puntero_pcb);

		while (tripulante_io->tarea->tiempo != 0)
		{
			tripulante_io->tarea->tiempo -= 1;
			sleep(RETARDO_CICLO_CPU);
			verificar_existencia_de_sabotaje();
		}

		buscar_tarea_a_RAM(tripulante_io);
		if (string_equals_ignore_case(tripulante_io->tarea->accion, "NULL"))
		{
			//si no tiene lo mando a EXIT
			log_info(logger, "TERMINE soy tripu %d de patota %d", tripulante_io->tid, tripulante_io->puntero_pcb);
			// add_queue(_EXIT_, list_remove(BLOCKED, 0)); //si se utilizar un GET para sacar de BLOCK luego debo remover el tripulante y pasarlo a EXIT
			mover_tripulante_entre_listas_si_existe(_BLOCKED_, _EXIT_, tripulante_io->puntero_pcb, tripulante_io->tid);
		}
		else
		{
			//si tiene lo envio a READY
			mover_tripulante_entre_listas_si_existe(_BLOCKED_, _READY_, tripulante_io->puntero_pcb, tripulante_io->tid);
			// add_queue(_READY_, list_remove(BLOCKED, 0)); //si se utilizar un GET para sacar de BLOCK luego debo remover el tripulante y pasarlo a READY
			// sem_post(&sem_exe_notificacion);
		}
		log_info(logger,"%s","Ahora si puede entrar el otro gil");

		// if (list_size(BLOCKED) > 0)
		// 	sem_post(&sem_IO);
		pthread_mutex_unlock(&mutex_entrada_salida);
	// }
}

void realizar_tarea_exe(t_tripulante *tripulante)
{

	//compruebo que tipo de tarea es (E/S o común)
	if (tripulante->tarea->parametro == -1)
	{
		puts("Peticion de E/S");
		sleep(1);
	}
	else
	{

		int movimientos = tripulante->posicion_x + tripulante->posicion_y;

		for (size_t i = 0; i < movimientos; i++)
		{
			log_info(logger, "Tripu %d de Patota %d, me faltan %d pasos para llegar a mi tarea",
					 tripulante->tid, tripulante->puntero_pcb, movimientos - i);
			//sacar
			sleep(1);
		}
		log_info(logger, "Realizando tarea %s de duracion %d", tripulante->tarea->accion, tripulante->tarea->tiempo);
		sleep(tripulante->tarea->tiempo);
		log_info(logger, "Tripu %d de Patota %d. Tarea realizada!", tripulante->tid, tripulante->puntero_pcb);
	}
}

void mover_a_la_posicion_de_la_tarea(t_tripulante *tripulante)
{
	int socket;
	while (tripulante->posicion_x < tripulante->tarea->posicion_x)
	{
		if (tripulante->estado == 'E')
		{
			socket = crear_conexion(IP_MI_RAM_HQ, PUERTO_MI_RAM_HQ);
			log_info(logger, "%d-%d me muevo de (%d,%d) a (%d,%d)",
					 tripulante->tid, tripulante->puntero_pcb, tripulante->posicion_x, tripulante->posicion_y,
					 tripulante->posicion_x + 1, tripulante->posicion_y);
			tripulante->posicion_x = tripulante->posicion_x + 1;
			enviar_posicion_a_ram(tripulante, socket);

			liberar_conexion(socket);
			sleep(RETARDO_CICLO_CPU);
			verificar_existencia_de_sabotaje();
			verificar_existencia_de_pausado();
		}
		// enviar_posicion_a_ram(tripulante, socket);
		if (tripulante->estado == 'F')
			pthread_exit((void *)pthread_self());
	}

	while (tripulante->posicion_x > tripulante->tarea->posicion_x)
	{
		if (tripulante->estado == 'E')
		{
			socket = crear_conexion(IP_MI_RAM_HQ, PUERTO_MI_RAM_HQ);
			log_info(logger, "%d-%d me muevo de (%d,%d) a (%d,%d)",
					 tripulante->tid, tripulante->puntero_pcb, tripulante->posicion_x, tripulante->posicion_y,
					 tripulante->posicion_x - 1, tripulante->posicion_y);
			tripulante->posicion_x = tripulante->posicion_x - 1;
			enviar_posicion_a_ram(tripulante, socket);
			liberar_conexion(socket);
			sleep(RETARDO_CICLO_CPU);
			verificar_existencia_de_sabotaje();
			verificar_existencia_de_pausado();
		}
		if (tripulante->estado == 'F')
			pthread_exit((void *)pthread_self());
	}

	while (tripulante->posicion_y < tripulante->tarea->posicion_y)
	{
		if (tripulante->estado == 'E')
		{
			socket = crear_conexion(IP_MI_RAM_HQ, PUERTO_MI_RAM_HQ);
			log_info(logger, "%d-%d me muevo de (%d,%d) a (%d,%d)",
					 tripulante->tid, tripulante->puntero_pcb, tripulante->posicion_x, tripulante->posicion_y,
					 tripulante->posicion_x, tripulante->posicion_y + 1);
			tripulante->posicion_y = tripulante->posicion_y + 1;
			enviar_posicion_a_ram(tripulante, socket);
			liberar_conexion(socket);
			sleep(RETARDO_CICLO_CPU);

			verificar_existencia_de_sabotaje();
			verificar_existencia_de_pausado();
		}
		// if(strcmp(&tripulante->estado, "E") == 0) enviar_posicion_a_ram(tripulante, socket);
		if (tripulante->estado == 'F')
			pthread_exit((void *)pthread_self());
		// if((strcmp(&tripulante->estado, "F") == 0)) pthread_exit((void*) pthread_self());
	}

	while (tripulante->posicion_y > tripulante->tarea->posicion_y)
	{
		if (tripulante->estado == 'E')
		{
			socket = crear_conexion(IP_MI_RAM_HQ, PUERTO_MI_RAM_HQ);
			log_info(logger, "%d-%d me muevo de (%d,%d) a (%d,%d)",
					 tripulante->tid, tripulante->puntero_pcb, tripulante->posicion_x, tripulante->posicion_y,
					 tripulante->posicion_x, tripulante->posicion_y - 1);
			tripulante->posicion_y = tripulante->posicion_y - 1;
			enviar_posicion_a_ram(tripulante, socket);
			liberar_conexion(socket);
			sleep(RETARDO_CICLO_CPU);

			verificar_existencia_de_sabotaje();
			verificar_existencia_de_pausado();
		}
		if (tripulante->estado == 'F')
			pthread_exit((void *)pthread_self());
	}
}

void expulsar_si_no_hay_tarea(t_tripulante *tripu)
{
	if (string_equals_ignore_case(tripu->tarea->accion, "NULL"))
	{
		//si no existe proxima tarea lo finalizo
		log_info(logger, "FINALICE %d-%d", tripu->tid, tripu->puntero_pcb);
		mover_tripulante_entre_listas_si_existe(_EXEC_, _EXIT_, tripu->puntero_pcb, tripu->tid);
		// if (list_size(READY) >= 1) sem_post(&sem_exe);
	}
}

void realizar_tarea_comun(t_tripulante *tripulante)
{
	if (es_tarea_comun(tripulante))
	{
		log_info(logger, "Tripu %d de Patota %d, realizando mi tarea %s que me lleva %d segundos",
				 tripulante->tid, tripulante->puntero_pcb, tripulante->tarea->accion,
				 tripulante->tarea->tiempo);
		while (tripulante->tarea->tiempo != 0)
		{
			// log_info(logger, "%d", tripulante->tarea->tiempo);
			tripulante->tarea->tiempo -= 1;
			sleep(RETARDO_CICLO_CPU);
		}
	}
}

bool es_tarea_comun(t_tripulante *tripulante)
{
	return tripulante->tarea->parametro == -1;
}

void peticion_ES(t_tripulante *tripulante)
{
	log_info(logger, "Peticion de E/S en EXE de %d-%d", tripulante->tid, tripulante->puntero_pcb);
	sleep(1);
}

void buscar_proxima_a_RAM_o_realizar_peticion_de_entrada_salida(t_tripulante *tripulante)
{
	if (es_tarea_comun(tripulante))
	{
		free(tripulante->tarea->accion);
		free(tripulante->tarea);
		buscar_tarea_a_RAM(tripulante);
	}
	else
	{
		peticion_ES(tripulante);
		// if (list_size(BLOCKED) == 0)
		// 	sem_post(&sem_IO);
		mover_tripulante_entre_listas_si_existe(_EXEC_, _BLOCKED_, tripulante->puntero_pcb, tripulante->tid);
		// sem_post(&sem_IO_queue);

		entrada_salida();
	}
}
