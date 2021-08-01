#include "discordiador.h"

void planificar_RR(t_tcb *tripulante)
{
    //buscar tarea inicial a RAM
    buscar_tarea_a_RAM(tripulante);

    //pasar a ready (crear funcioon que setea la variable estado del hilo y agregarlo a la cola de READY)
    add_queue(_READY_, tripulante);

    while (1)
    {                       //para pausar la planificacion, en accion de PAUSAR_PLANIFICACION se decrementara un semaforo que estará en funciones de exe y entrada_salida
        sem_wait(&sem_exe); //semaforo de multiprocesamiento
        log_info(logger, "EJECUTANDO Tripulante %d de patota %d", tripulante->tid, tripulante->puntero_pcb);
        //sacar de lista de READY y pasar a EXE
        pthread_mutex_lock(&mutex_lista_ready);
        t_tcb *tripulante_exe = list_remove(READY, 0);
        pthread_mutex_unlock(&mutex_lista_ready);
        add_queue(_EXEC_, tripulante_exe);

        tripulante_exe->QUANTUM_ACTUAL = 5;
        while (tripulante_exe->QUANTUM_ACTUAL > 0)
        {
            moverme_hacia_tarea_RR(tripulante_exe);
            realizar_tarea_comun_RR(tripulante_exe);

            if (tripulante_exe->QUANTUM_ACTUAL == -1)
            {
                if (!es_tarea_comun(tripulante_exe))
                {
                    pthread_mutex_lock(&mutex_lista_exec);
                   mover_tripulante_entre_listas_si_existe(_EXEC_,_BLOCKED_,tripulante_exe->puntero_pcb,tripulante_exe->tid);
                    pthread_mutex_unlock(&mutex_lista_exec);
                    sem_post(&sem_IO_queue);
                    if ((list_size(EXEC) < GRADO_MULTITAREA && list_size(READY) > 0))
                        sem_post(&sem_exe);
                    if (list_size(BLOCKED) > 0)
                        sem_post(&sem_IO);
                    break;
                }
                else
                {
                    buscar_tarea_a_RAM(tripulante_exe);
                    expulsar_si_no_hay_tarea(tripulante_exe);
                }
            }
        }

        pthread_mutex_lock(&mutex_lista_exec);
        mover_tripulante_entre_listas_si_existe(_EXEC_,_READY_,tripulante_exe->puntero_pcb,tripulante_exe->tid);
        pthread_mutex_unlock(&mutex_lista_exec);
        sem_post(&sem_exe);
    }
}

void realizar_tarea_comun_RR(t_tcb *tripulante)
{

    if (!(tripulante->tarea->posicion_x == tripulante->posicion_x && tripulante->tarea->posicion_y == tripulante->posicion_y) || tripulante->QUANTUM_ACTUAL <= 0)
    {
        return ;
    }

    if (tripulante->tarea->parametro == -1)
    {

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
            if (tripulante->QUANTUM_ACTUAL >= 0 && tripulante->tarea->tiempo == 0)
            {
                //if(tiempo_tarea==(i+1)){
                tripulante->QUANTUM_ACTUAL = -1; //me llevo que terminó la tarea cuando su Q es 0
                sleep(1);
                log_info(logger, "Tarea %s realizada tripu %d de Patota %d",
                         tripulante->tarea->accion, tripulante->tid, tripulante->puntero_pcb);
                return;
                //}
            }

            sleep(1);
        }
    }else{
                if (tripulante->QUANTUM_ACTUAL > 0)
            {
                peticion_ES(tripulante);
                tripulante->QUANTUM_ACTUAL = tripulante->QUANTUM_ACTUAL -1;
            }else
            {
                return ;
            }

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
                return ;
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
                return ;
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
                return ;
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
                return ;
            }
            sleep(1);
        }
    }
}