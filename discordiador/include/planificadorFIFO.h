#include "discordiador.h"


void planificar_FIFO()
{

	sem_init(&semaforo_1, 0, 0); //la cantidad de recursos viene por archivo de configuracion (permite la multiprogramacion)
	sem_init(&sem_IO, 0, 1);
	pthread_barrier_init(&barrera,NULL,1);
	pthread_t hilo_exe[100];
	//pthread_t hilo_exe_IO;


	// int niver_multiprogramacion = 1;
	//while((!list_is_empty(READY) && !list_is_empty(BLOCKED)) || planificar)
	while(planificar)
	{	
		for(size_t i = 0; i < list_size(READY); i++)
		{
			pthread_create(&hilo_exe[i],NULL,&exe,NULL);
		}
				
		for(size_t j = 0; j < list_size(READY); j++)
		{
			pthread_join(hilo_exe[j],NULL);
		}
	}


}

void exe()
{	

	sem_wait(&sem_IO);
	puts("EJECUTANDO");
	pthread_mutex_lock(&mutex_lista_ready);
	t_tcb* tripulante_exe =list_remove(READY,0);
	pthread_mutex_unlock(&mutex_lista_ready);

	realizar_tarea_exe(tripulante_exe);

	//chequear si tiene que ir a block o salir

	if(tripulante_exe->posicion_x==0){ //en planificacion FIFO el tripulante va a realizar toda su ejecucion sin ser desalojado, luego, si corresponde irá a E/S realizando su petición previa.
		log_info(logger,"FIN de mi planificacion tripu %d patota %d",
		tripulante_exe->tid,tripulante_exe->puntero_pcb);
		if(list_is_empty(READY) && list_is_empty(BLOCKED)){planificar=false;}
		sem_post(&sem_IO);
		pthread_exit(pthread_self());
	}else{
		sem_post(&sem_IO);
		pthread_mutex_lock(&mutex_lista_new);
		list_add(BLOCKED,tripulante_exe);
		sem_post(&semaforo_1);
		pthread_mutex_unlock(&mutex_lista_new);
		}	


	sem_wait(&semaforo_1);
	puts("ENTRADA_SALIDA");
	sem_post(&sem_IO);
	pthread_mutex_lock(&mutex_lista_new);
	t_tcb* tripulante_blocked = list_remove(BLOCKED,0);
	pthread_mutex_unlock(&mutex_lista_new);
	
	entrada_salida(tripulante_blocked);

	pthread_mutex_lock(&mutex_lista_ready);
	list_add(READY,tripulante_blocked);
	pthread_mutex_unlock(&mutex_lista_ready);
	sem_post(&sem_IO);
}


void entrada_salida(t_tcb* tripulante)
{	
		for (size_t i = 0; i < tripulante->posicion_x; i++)
	{	
		log_info(logger,"Me quedan %d pasos para llegar a mi tarea soy hilo %d de patota %d",
		tripulante->posicion_x-i,tripulante->tid,tripulante->puntero_pcb);
		sleep(1);
	}
	
	log_info(logger,"Realizando tarea en E/S soy hilo %d de patota %d",
				tripulante->tid,tripulante->puntero_pcb);

	sleep(tripulante->posicion_x);
	log_info(logger,"Termine tarea de E/S soy hilo %d de patota %d",
				tripulante->tid,tripulante->puntero_pcb);

	tripulante->posicion_x = 0;

}


void realizar_tarea_exe(t_tcb* tripulante){
	int movimientos = tripulante->posicion_x + tripulante->posicion_y;
	for (size_t i = 0; i < movimientos; i++)
	{
		log_info(logger,"Tripu %d de Patota %d, me faltan %d pasos para llegar a mi tarea",
			tripulante->tid,tripulante->puntero_pcb,movimientos-i);
			sleep(1);
	}
	log_info(logger,"Realizando tarea %s de duracion %d",tripulante->tarea->accion,tripulante->tarea->tiempo);
	sleep(tripulante->tarea->tiempo);
	log_info(logger,"Tripu %d de Patota %d. Tarea realizada!",tripulante->tid,tripulante->puntero_pcb);
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
