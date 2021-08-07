#include "discordiador.h"

void planificar_RR(t_tripulante *tripulante)
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
        tripulante = list_remove(READY, 0); //se saca el primer tripulante de la cola de ready
        add_queue(_EXEC_, tripulante);
        tripulante->QUANTUM_ACTUAL = QUANTUM;

        do
        {
            moverme_hacia_tarea_RR(tripulante);
            realizar_tarea_comun_RR(tripulante);
            buscar_proxima_a_RAM_o_realizar_peticion_de_entrada_salida_RR(tripulante);
            expulsar_si_no_hay_tarea(tripulante);
            if (tripulante->estado == 'E' && tripulante->QUANTUM_ACTUAL <= 0)
            {
                mover_tripulante_entre_listas_si_existe(_EXEC_, _READY_, tripulante->puntero_pcb, tripulante->tid);
            }
        } while (tripulante->estado == 'E');
    }
}

void realizar_tarea_comun_RR(t_tripulante *tripulante)
{
    if (es_tarea_comun(tripulante))
    {
        // enviar_comienzo_o_finalizacion_de_tarea_a_imongo_store_para_BITACORA(tripulante,"Comienza ejecución de tarea ");
        log_info(logger, "Tripu %d de Patota %d, realizando mi tarea %s que me lleva %d segundos",
                 tripulante->tid, tripulante->puntero_pcb, tripulante->tarea->accion,
                 tripulante->tarea->tiempo);
        while (tripulante->tarea->tiempo != 0)
        {
            if (tripulante->QUANTUM_ACTUAL <= 0)
                return;
            log_info(logger, "%d", tripulante->tarea->tiempo);
            verificar_existencia_de_sabotaje();
            verificar_existencia_de_pausado();
            tripulante->tarea->tiempo -= 1;
            tripulante->QUANTUM_ACTUAL -= 1;
            sleep(RETARDO_CICLO_CPU);
        }
        // enviar_comienzo_o_finalizacion_de_tarea_a_imongo_store_para_BITACORA(tripulante,"Se finaliza la tarea ");
    }
}

void moverme_hacia_tarea_RR(t_tripulante *tripulante)
{
    //se envia la posicion inicial del tripulante y la coordenada de la tarea

    // enviar_movimiento_a_imongo_store_para_BITACORA(tripulante);

    do
    {
        int socket;
        while (tripulante->posicion_x < tripulante->tarea->posicion_x)
        {
            if (tripulante->estado == 'E')
            {
                if (tripulante->QUANTUM_ACTUAL <= 0)
                    return;
                log_info(logger, "%d-%d me muevo de (%d,%d) a (%d,%d)",
                         tripulante->tid, tripulante->puntero_pcb, tripulante->posicion_x, tripulante->posicion_y,
                         tripulante->posicion_x + 1, tripulante->posicion_y);
                tripulante->posicion_x = tripulante->posicion_x + 1;
                tripulante->QUANTUM_ACTUAL -= 1;
                verificar_existencia_de_sabotaje();
                verificar_existencia_de_pausado();
                socket = crear_conexion(IP_MI_RAM_HQ, PUERTO_MI_RAM_HQ);
                enviar_posicion_a_ram(tripulante, socket);
                liberar_conexion(socket);
                sleep(RETARDO_CICLO_CPU);
            }
            // enviar_posicion_a_ram(tripulante, socket);
            if (tripulante->estado == 'F')
                pthread_exit((void *)pthread_self());
        }

        while (tripulante->posicion_x > tripulante->tarea->posicion_x)
        {
            if (tripulante->estado == 'E')
            {
                if (tripulante->QUANTUM_ACTUAL <= 0)
                    return;

                log_info(logger, "%d-%d me muevo de (%d,%d) a (%d,%d)",
                         tripulante->tid, tripulante->puntero_pcb, tripulante->posicion_x, tripulante->posicion_y,
                         tripulante->posicion_x - 1, tripulante->posicion_y);
                tripulante->posicion_x = tripulante->posicion_x - 1;
                tripulante->QUANTUM_ACTUAL -= 1;
                verificar_existencia_de_sabotaje();
                verificar_existencia_de_pausado();
                socket = crear_conexion(IP_MI_RAM_HQ, PUERTO_MI_RAM_HQ);
                enviar_posicion_a_ram(tripulante, socket);
                liberar_conexion(socket);
                sleep(RETARDO_CICLO_CPU);
            }
            if (tripulante->estado == 'F')
                pthread_exit((void *)pthread_self());
        }

        while (tripulante->posicion_y < tripulante->tarea->posicion_y)
        {
            if (tripulante->estado == 'E')
            {
                if (tripulante->QUANTUM_ACTUAL <= 0)
                    return;

                log_info(logger, "%d-%d me muevo de (%d,%d) a (%d,%d)",
                         tripulante->tid, tripulante->puntero_pcb, tripulante->posicion_x, tripulante->posicion_y,
                         tripulante->posicion_x, tripulante->posicion_y + 1);
                tripulante->posicion_y = tripulante->posicion_y + 1;
                tripulante->QUANTUM_ACTUAL -= 1;
                verificar_existencia_de_sabotaje();
                verificar_existencia_de_pausado();
                socket = crear_conexion(IP_MI_RAM_HQ, PUERTO_MI_RAM_HQ);
                enviar_posicion_a_ram(tripulante, socket);
                liberar_conexion(socket);
                sleep(RETARDO_CICLO_CPU);
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
                if (tripulante->QUANTUM_ACTUAL <= 0)
                    return;

                log_info(logger, "%d-%d me muevo de (%d,%d) a (%d,%d)",
                         tripulante->tid, tripulante->puntero_pcb, tripulante->posicion_x, tripulante->posicion_y,
                         tripulante->posicion_x, tripulante->posicion_y - 1);
                tripulante->posicion_y = tripulante->posicion_y - 1;
                tripulante->QUANTUM_ACTUAL -= 1;
                verificar_existencia_de_sabotaje();
                verificar_existencia_de_pausado();
                socket = crear_conexion(IP_MI_RAM_HQ, PUERTO_MI_RAM_HQ);
                enviar_posicion_a_ram(tripulante, socket);
                liberar_conexion(socket);
                sleep(RETARDO_CICLO_CPU);
            }
            if (tripulante->estado == 'F')
                pthread_exit((void *)pthread_self());
        }
    } while (!(tripulante->posicion_x == tripulante->tarea->posicion_x && tripulante->posicion_y == tripulante->tarea->posicion_y));
}

void buscar_proxima_a_RAM_o_realizar_peticion_de_entrada_salida_RR(t_tripulante *tripulante)
{
    if (tripulante->QUANTUM_ACTUAL <= 0)
        return;
    if (es_tarea_comun(tripulante))
    {
        free(tripulante->tarea->accion);
        free(tripulante->tarea);
        verificar_existencia_de_sabotaje();
        verificar_existencia_de_pausado();
        buscar_tarea_a_RAM(tripulante);
    }
    else
    {
        peticion_ES_RR(tripulante);

        mover_tripulante_entre_listas_si_existe(_EXEC_, _BLOCKED_, tripulante->puntero_pcb, tripulante->tid);

        entrada_salida();
    }
}

void peticion_ES_RR(t_tripulante *tripulante)
{
    if (tripulante->QUANTUM_ACTUAL <= 0)
        return;
    // enviar_tarea_de_ES_a_imongostore(tripulante);
    verificar_existencia_de_sabotaje();
    verificar_existencia_de_pausado();

    log_info(logger, "Peticion de E/S en EXE de %d-%d", tripulante->tid, tripulante->puntero_pcb);
    sleep(RETARDO_CICLO_CPU);
}