#include "discordiador.h"

void planificar_FIFO(t_tcb *tripulante)
{

	log_info(logger, "Tripulante %d de patota %d", tripulante->tid, tripulante->puntero_pcb);
	//pthread_barrier_wait(&barrera);

	//buscar tarea inicial a RAM
	buscar_tarea_a_RAM(tripulante);

	//pasar a ready (crear funcioon que setea la variable estado del hilo y agregarlo a la cola de READY)
	add_queue(_READY_, tripulante);

	if (primerIntento)
	{
		pthread_cond_wait(&iniciarPlanificacion, &mutex);
	}
	//barrera que arranca y se libera en INICIAR_PLANIFICACION (aca todos los hilos van a estar esperando)

	while (1)
	{ //para pausar la planificacion, en accion de PAUSAR_PLANIFICACION se decrementara un semaforo que estará en funciones de exe y entrada_salida

		sem_wait(&sem_exe); //semaforo de multiprocesamiento
		puts("EJECUTANDO");
		pthread_mutex_lock(&mutex_lista_ready);
		//sacar de lista de READY y pasar a EXE
		t_tcb *tripulante_exe = list_remove(READY, 0);
		add_queue(_EXEC_, tripulante_exe);
		pthread_mutex_unlock(&mutex_lista_ready);

		//moverme hacia la tarea
		int movimientos = tripulante_exe->posicion_x + tripulante->posicion_y;
		for (size_t i = 0; i < movimientos; i++)
		{
			log_info(logger, "Tripu %d de Patota %d, me faltan %d pasos para llegar a mi tarea",
					 tripulante_exe->tid, tripulante_exe->puntero_pcb, movimientos - i);
			//sacar
			sleep(1);
		}
		log_info(logger, "Tripu %d de Patota %d, llegué a %s ",tripulante_exe->tid, tripulante_exe->puntero_pcb,tripulante_exe->tarea->accion);
		//compruebo que tipo de tarea es (E/S o común(-1))
		if (tripulante_exe->tarea->parametro == -1)
		{
			//comun---> ejecuto lo que indica el tiempo y luego busco la proxima tarea
			//ejecuto
			//realizando tarea
			int tiempo_tarea = tripulante_exe->tarea->tiempo;
			for (size_t i = 0; i < tiempo_tarea; i++)
		{
			log_info(logger, "Tripu %d de Patota %d, realizando mi tarea %s quedan %d",
					 tripulante_exe->tid, tripulante_exe->puntero_pcb,tripulante_exe->tarea->accion,
					  tiempo_tarea-i);
			tripulante_exe->tarea->tiempo=tripulante_exe->tarea->tiempo - 1;
			//sacar
			sleep(1);
		}
			buscar_tarea_a_RAM(tripulante_exe);
			//chequeo si tiene proxima tarea
			if (tripulante_exe->tarea->accion == NULL)
			{
				//si no tiene lo mando a EXIT
				pthread_mutex_lock(&mutex_lista_ready);
				add_queue(_EXIT_, remover_tripu(EXEC, tripulante_exe->tid));
				pthread_mutex_unlock(&mutex_lista_ready);
				sem_post(&sem_exe);
				pthread_exit(pthread_self);
			}
			else
			{
				//si tiene lo envio a READY
				pthread_mutex_lock(&mutex_lista_ready);
				add_queue(_READY_, remover_tripu(EXEC, tripulante_exe->tid));
				pthread_mutex_unlock(&mutex_lista_ready);
				sem_post(&sem_exe);
			}
		}
		else
		{
			//E/S ---> lo agrego a la lista de BLOCK
			puts("Peticion de E/S en EXE");
			sleep(1);
			pthread_mutex_lock(&mutex_lista_ready);
			add_queue(_BLOCKED_, remover_tripu(EXEC, tripulante_exe->tid));
			pthread_mutex_unlock(&mutex_lista_ready);
			sem_post(&sem_exe);
			sem_post(&sem_IO_queue);
		}
	}
}

void exe(t_tcb tripulante)
{
}

void entrada_salida()
{
	while (1)
	{
		sem_wait(&sem_IO_queue);
		sem_wait(&sem_IO); //semaforo de entrada salida
		pthread_mutex_lock(&mutex_lista_ready);
		//sacar de lista de BLOCKED y ejecutar
		t_tcb *tripulante_io = list_remove(BLOCKED, 0);
		pthread_mutex_unlock(&mutex_lista_ready);

		puts("EJECUTANDO IO");
		sleep(tripulante_io->tarea->tiempo);

		buscar_tarea_a_RAM(tripulante_io);
		if (tripulante_io->tarea->accion == NULL)
		{
			//si no tiene lo mando a EXIT
			pthread_mutex_lock(&mutex_lista_ready);
			add_queue(_EXIT_, tripulante_io);
			pthread_mutex_unlock(&mutex_lista_ready);
			sem_post(&sem_IO);
			//pthread_exit(pthread_self);
		}
		else
		{
			//si tiene lo envio a READY
			pthread_mutex_lock(&mutex_lista_ready);
			add_queue(_READY_, tripulante_io);
			pthread_mutex_unlock(&mutex_lista_ready);
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

bool es_tarea_de_ES(char *accion)
{
	if (string_contains(accion, " "))
	{
		return true;
	}
	else
	{
		return false;
	}
}
