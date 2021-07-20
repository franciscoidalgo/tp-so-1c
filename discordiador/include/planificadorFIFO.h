#include "discordiador.h"

void planificar_FIFO(t_tcb *tripulante)
{
	//buscar tarea inicial a RAM
	buscar_tarea_a_RAM(tripulante);

	//pasar a ready (crear funcioon que setea la variable estado del hilo y agregarlo a la cola de READY)
	add_queue(_READY_, tripulante);

	while (1)
	{ //para pausar la planificacion, en accion de PAUSAR_PLANIFICACION se decrementara un semaforo que estará en funciones de exe y entrada_salida

		sem_wait(&sem_exe); //semaforo de multiprocesamiento
		log_info(logger, "EJECUTANDO Tripulante %d de patota %d", tripulante->tid, tripulante->puntero_pcb);
		//sacar de lista de READY y pasar a EXE
		pthread_mutex_lock(&mutex_lista_ready);
		t_tcb *tripulante_exe = list_remove(READY, 0);
		pthread_mutex_unlock(&mutex_lista_ready);
		add_queue(_EXEC_, tripulante_exe);

		//moverme hacia la tarea
		moverme_hacia_tarea(tripulante_exe);

		//compruebo que tipo de tarea es (E/S o común(-1))
		if (es_tarea_comun(tripulante_exe))
		{

			realizar_tarea_comun(tripulante_exe);
			buscar_tarea_a_RAM(tripulante_exe);
			consultar_proxima_tarea(tripulante_exe); //si existe lo paso a READY sino a EXIT
		}
		else
		{

			peticion_ES(tripulante_exe); //realizo la peticion y lo paso a BLOCKED

			pthread_mutex_lock(&mutex_lista_exec);
			add_queue(_BLOCKED_, remover_tripu(EXEC, tripulante_exe->tid));
			pthread_mutex_unlock(&mutex_lista_exec);
			sem_post(&sem_IO_queue);
			if (list_size(READY) < 2)
			{	sem_wait(&sem_exe);
				sem_post(&sem_exe);
			}
		}
	}
}

void entrada_salida()
{
	while (1)
	{
		sem_wait(&sem_IO_queue);
		puts("DISPOSITIVO DE ENTRADA SALIDA");
		//pthread_mutex_lock(&mutex);
		sem_wait(&sem_IO); //semaforo de entrada salida
		//sacar de lista de BLOCKED y ejecutar
		t_tcb *tripulante_io = list_remove(BLOCKED, 0);
		//pthread_mutex_unlock(&mutex_lista_ready);

		log_info(logger, "EJECUTANDO IO soy tripu %d de patota %d", tripulante_io->tid, tripulante_io->puntero_pcb);
		sleep(tripulante_io->tarea->tiempo);

		buscar_tarea_a_RAM(tripulante_io);
		if (string_equals_ignore_case(tripulante_io->tarea->accion, "NULL"))
		{
			//si no tiene lo mando a EXIT
			log_info(logger, "TERMINE soy tripu %d de patota %d", tripulante_io->tid, tripulante_io->puntero_pcb);
			pthread_mutex_lock(&mutex_lista_ready);
			add_queue(_EXIT_, tripulante_io);
			pthread_mutex_unlock(&mutex_lista_ready);
			sem_post(&sem_IO);
			if (list_size(BLOCKED) == 0 && list_size(EXEC) == 0 && list_size(READY) == 0)
			{
				pthread_exit(pthread_self);
			}
		}
		else
		{
			//si tiene lo envio a READY
			pthread_mutex_lock(&mutex_lista_ready);
			add_queue(_READY_, tripulante_io);
			pthread_mutex_unlock(&mutex_lista_ready);
			sem_post(&sem_exe);
			sem_post(&sem_IO);
		}
	}
}

void realizar_tarea_exe(t_tcb *tripulante)
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

void moverme_hacia_tarea(t_tcb *tripulante)
{
	int cantidad_de_pasos_en_x = abs(tripulante->posicion_x - tripulante->tarea->posicion_x);
	int cantidad_de_pasos_en_y = abs(tripulante->posicion_y - tripulante->tarea->posicion_y);

	if (tripulante->posicion_x < tripulante->tarea->posicion_x)
	{
		for (size_t i = 0; i < cantidad_de_pasos_en_x; i++)
		{
			log_info(logger, "%d-%d me muevo de (%d,%d) a (%d,%d)",
					 tripulante->puntero_pcb, tripulante->tid, tripulante->posicion_x, tripulante->posicion_y,
					 tripulante->posicion_x + 1, tripulante->posicion_y);
			tripulante->posicion_x = tripulante->posicion_x + 1;
			sleep(1);
		}
	}

	if (tripulante->posicion_x > tripulante->tarea->posicion_x)
	{
		for (size_t i = 0; i < cantidad_de_pasos_en_x; i++)
		{
			log_info(logger, "%d-%d me muevo de (%d,%d) a (%d,%d)",
					 tripulante->puntero_pcb, tripulante->tid, tripulante->posicion_x, tripulante->posicion_y,
					 tripulante->posicion_x - 1, tripulante->posicion_y);
			tripulante->posicion_x = tripulante->posicion_x - 1;
			sleep(1);
		}
	}

	if (tripulante->posicion_y < tripulante->tarea->posicion_y)
	{
		for (size_t i = 0; i < cantidad_de_pasos_en_y; i++)
		{
			log_info(logger, "%d-%d me muevo de (%d,%d) a (%d,%d)",
					 tripulante->puntero_pcb, tripulante->tid, tripulante->posicion_x, tripulante->posicion_y,
					 tripulante->posicion_x, tripulante->posicion_y + 1);
			tripulante->posicion_y = tripulante->posicion_y + 1;
			sleep(1);
		}
	}

	if (tripulante->posicion_y > tripulante->tarea->posicion_y)
	{
		for (size_t i = 0; i < cantidad_de_pasos_en_y; i++)
		{
			log_info(logger, "%d-%d me muevo de (%d,%d) a (%d,%d)",
					 tripulante->puntero_pcb, tripulante->tid, tripulante->posicion_x, tripulante->posicion_y,
					 tripulante->posicion_x, tripulante->posicion_y - 1);
			tripulante->posicion_y = tripulante->posicion_y - 1;
			sleep(1);
		}
	}

	log_info(logger, "%d-%d llegué a la tarea", tripulante->puntero_pcb, tripulante->tid);
}

void consultar_proxima_tarea(t_tcb *tripu)
{
	if (string_equals_ignore_case(tripu->tarea->accion, "NULL"))
	{
		//si no existe proxima tarea lo finalizo
		pthread_mutex_lock(&mutex_lista_ready);
		log_info(logger, "FINALICE %d-%d", tripu->tid, tripu->puntero_pcb);
		add_queue(_EXIT_, remover_tripu(EXEC, tripu->tid));
		pthread_mutex_unlock(&mutex_lista_ready);
		sem_post(&sem_exe);
		pthread_cancel(pthread_self());
	}
	else
	{
		//si tiene lo envio al final de la cola de READY
		add_queue(_READY_, remover_tripu(EXEC, tripu->tid));
		sem_post(&sem_exe);
	}
}

void realizar_tarea_comun(t_tcb *tripulante)
{
	int tiempo_tarea = tripulante->tarea->tiempo;
	for (size_t i = 0; i < tiempo_tarea; i++)
	{
		log_info(logger, "Tripu %d de Patota %d, realizando mi tarea %s quedan %d",
				 tripulante->tid, tripulante->puntero_pcb, tripulante->tarea->accion,
				 tiempo_tarea - i);
		tripulante->tarea->tiempo = tripulante->tarea->tiempo - 1;
		//sacar
		sleep(1);
	}
}

bool es_tarea_comun(t_tcb *tripulante)
{
	return tripulante->tarea->parametro == -1;
}

void peticion_ES(t_tcb *tripulante)
{
	log_info(logger, "Peticion de E/S en EXE de %d-%d", tripulante->tid, tripulante->puntero_pcb);
	sleep(2);
}