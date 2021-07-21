#include "discordiador.h"

void planificar_RR(t_tcb *tripulante)
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
        tripulante_exe->QUANTUM_ACTUAL = 2;
        moverme_hacia_tarea_RR(tripulante_exe);

        //compruebo que tipo de tarea es (E/S o común(-1))
        if (es_tarea_comun(tripulante_exe))
        {
            realizar_tarea_comun_RR(tripulante_exe);

            if(tripulante_exe->QUANTUM_ACTUAL == -1){
            buscar_tarea_a_RAM(tripulante_exe);
            consultar_proxima_tarea(tripulante_exe); //si existe lo paso a READY sino a EXIT
            }else{
            pthread_mutex_lock(&mutex_lista_exec);
            add_queue(_READY_, remover_tripu(EXEC, tripulante_exe->tid));
            pthread_mutex_unlock(&mutex_lista_exec);
            sem_post(&sem_exe);
            }
        }
        else
        {
            if(tripulante_exe->QUANTUM_ACTUAL > 0){
            peticion_ES(tripulante_exe); //realizo la peticion y lo paso a BLOCKED
            pthread_mutex_lock(&mutex_lista_exec);
            add_queue(_BLOCKED_, remover_tripu(EXEC, tripulante_exe->tid));
            pthread_mutex_unlock(&mutex_lista_exec);
            sem_post(&sem_IO_queue);
            if (list_size(READY)+1 < NIVEL_MULTIPROCESAMIENTO)
            {
                sem_init(&sem_exe, 0, 0);
            }
            }else
            {
            pthread_mutex_lock(&mutex_lista_exec);
            add_queue(_READY_, remover_tripu(EXEC, tripulante_exe->tid));
            pthread_mutex_unlock(&mutex_lista_exec);
            sem_post(&sem_exe);
            }            
        }
    }
}

void realizar_tarea_comun_RR(t_tcb *tripulante)
{

    if(!(tripulante->tarea->posicion_x==tripulante->posicion_x && tripulante->tarea->posicion_y==tripulante->posicion_y) || tripulante->QUANTUM_ACTUAL <=0){
            return 0;
    }
    
    //int tiempo_tarea = tripulante->tarea->tiempo;
    int Q = tripulante->QUANTUM_ACTUAL;
    for (size_t i = 1; i <= Q; i++)
    {
        log_info(logger, "Tripu %d de Patota %d, realizando mi tarea %s quedan %d",
                 tripulante->tid, tripulante->puntero_pcb, tripulante->tarea->accion,
                 tripulante->tarea->tiempo);
        tripulante->tarea->tiempo = tripulante->tarea->tiempo - 1;
        //sacar
        tripulante->QUANTUM_ACTUAL = tripulante->QUANTUM_ACTUAL - 1;
        //comprobar QUANTUM
        if (tripulante->QUANTUM_ACTUAL >= 0 && tripulante->tarea->tiempo==0)
        {   
            //if(tiempo_tarea==(i+1)){
            tripulante->QUANTUM_ACTUAL = -1;//me llevo que terminó la tarea cuando su Q es 0
            sleep(1);
            log_info(logger, "Tarea %s realizada tripu %d de Patota %d",
            tripulante->tarea->accion,tripulante->tid, tripulante->puntero_pcb);
            return 0;
            //}

        }

        sleep(1);
    }
    
}

void moverme_hacia_tarea_RR(t_tcb *tripulante)
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

            tripulante->QUANTUM_ACTUAL = tripulante->QUANTUM_ACTUAL - 1;
            //comprobar QUANTUM
            if (tripulante->QUANTUM_ACTUAL == 0)
            {
                sleep(1);
                 return 0;
            }

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

            tripulante->QUANTUM_ACTUAL = tripulante->QUANTUM_ACTUAL - 1;
            //comprobar QUANTUM
            if (tripulante->QUANTUM_ACTUAL == 0)
            {
                sleep(1);
                return 0;
            }


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

            tripulante->QUANTUM_ACTUAL = tripulante->QUANTUM_ACTUAL - 1;
            //comprobar QUANTUM
            if (tripulante->QUANTUM_ACTUAL == 0)
            {
                sleep(1);
                 return 0;
            }



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

            tripulante->QUANTUM_ACTUAL = tripulante->QUANTUM_ACTUAL - 1;
            //comprobar QUANTUM
            if (tripulante->QUANTUM_ACTUAL == 0)
            {
                sleep(1);
               return 0;
            }
            sleep(1);
        }
    }

}