#include "miramhq.h"

#define SMOBJ_MEMORIA "/miramhq"

t_log* logger;
t_config* config;
administrador_de_segmentacion* admin_segmentacion;
t_list* lista_de_patotas;
pthread_mutex_t sem_memoria, mutex_segmentos_libres;
int SMOBJ_SIZE;
int cod_cierre;

/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */


int main(int argc, char ** argv){


    /* -------------- Variables / Estructuras Adminisdtrativas ---------------- */
    // variables - instanciacion
    // int conexion;
    // char* mensaje;
    
    // Estructuras Administrativas
    // creacion del logger y del config
    logger = iniciar_logger("miramhq");
    config = leer_config("miramhq");

    // imprime si alguno de los dos no se pudo crear
    validar_logger(logger);
    validar_config(config);

    /* -------------------------Primer Paso----------------------------------- */
    // Reservar espacio de memoria
    reservar_espacio_de_memoria();

    iniciar_segmentacion();

    printf("\tCantidad de bytes libres: %d\n",admin_segmentacion->bytes_libres);
    list_iterate(admin_segmentacion->segmentos_libres, (void*) iterator_segmento);
    printf("\n");

    // testear_asignar_y_liberar_segmentacion();


    // liberar_espacio_de_memoria();
    /* -----------------------Fin Primer Paso--------------------------------- */

    // conexion = crear_conexion((char*)config_get_string_value(config,"IP"),(char*)config_get_string_value(config,"PUERTO"));
    
        // Testeos
    // testear_biblioteca_compartida();
    // validar_malloc();
    // testear_enviar_mensaje(conexion);  //enviar un mje con una conexion al servidor

    /* -------------------------Segundo Paso----------------------------------- */
    // Crear el mapa

    /* -----------------------Fin Segundo Paso-------------------------------- */

    /* -------------------------Tercer Paso----------------------------------- */
    // loggear_linea();
    // t_segmento* seg_1, *seg_2, *seg_3, *seg_4, *seg_5, *seg_6, *seg_7;
    // seg_1 = buscar_segmento_libre(20);
    // seg_2 = buscar_segmento_libre(21);
    // seg_3 = buscar_segmento_libre(59);
    // seg_4 = buscar_segmento_libre(81);
    // seg_5 = buscar_segmento_libre(119);

    // t_list* lista_ocupada = list_create();
    // list_add(lista_ocupada,seg_5);
    // list_add(lista_ocupada,seg_3);
    // list_add(lista_ocupada,seg_1);
    // list_sort(lista_ocupada,orden_lista_segmentos);
    // liberar_segmento(seg_2);
    // liberar_segmento(seg_4);

    // liberar_segmento(seg_1);
    // liberar_segmento(seg_3);
    // liberar_segmento(seg_5);

    // log_info(logger,"\tSegmentos ocupados");
    // list_iterate(lista_ocupada, (void*) iterator_segmento);
    
    // compactar(lista_ocupada);
    
    // Continuara

    // Iniciar Servidor
    int server_fd = iniciar_servidor(logger,config);

    signal(SIGCHLD,&my_signal_kill);
    signal(SIGHUP,&my_signal_compactar);
    cod_cierre = 0;

    pthread_mutex_init(&sem_memoria,NULL);
    pthread_mutex_init(&mutex_segmentos_libres,NULL);

    while(cod_cierre == 0)
    {
        // imprimir_patotas_presentes();
        // Bloqueamos al servidor hasta que reciba algun connect
        int cliente_fd = esperar_cliente(server_fd,logger);

        // Crear el hilo aca con el cliente que acaba de llegar
        pthread_t un_hilo;
        pthread_create(&un_hilo,NULL, (void*) atender_cliente,cliente_fd);

        pthread_detach(un_hilo);
    }
    // list_clean_and_destroy_elements(lista, (void*) iterator_destroy);
    // list_iterate(lista, (void*) iterator_destroy);
    /* -----------------------Fin Tercer Paso-------------------------------- */

    // terminamos el proceso, eliminamos todo
    pthread_mutex_destroy(&sem_memoria);
    pthread_mutex_destroy(&mutex_segmentos_libres);
    free(MEMORIA); 
    terminar_miramhq(logger,config);

}






/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*FUNCIONES*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */

void atender_cliente(int cliente_fd)
{
    char* esquema = config_get_string_value(config,"ESQUEMA_MEMORIA");
    
    if ( strcmp(esquema,"SEGMENTACION") == 0)
    {
        atender_cliente_SEGMENTACION(cliente_fd);
    }else
    {
        log_info(logger,"PAGINACION");
        atender_cliente_SEGMENTACION(cliente_fd);
    }
}



void atender_cliente_SEGMENTACION(int cliente_fd)
{
        int cod_op = recibir_operacion(cliente_fd);
        
        t_list* lista = list_create();



        switch(cod_op)
		{
		case MENSAJE: ;
			int size;
			recibir_mensaje(cliente_fd,logger,&size);
            // void* buffer = recibir_buffer(&size, cliente_fd);
            // log_info(logger, "Me llego el mensaje %p", buffer);
            // free(buffer);
			break;
		case PAQUETE:
			// lista = recibir_paquete(cliente_fd);
			printf("Me llegaron los siguientes valores:\n");
			list_iterate(lista, (void*) iterator);
			break;
		case INICIAR_PATOTA:

            log_info(logger,"Una patota se ha iniciado");
			recibir_paquete(cliente_fd,lista);
            // crear_patota();

    /* ----------------------Estrucutras administrativas-------------------------- */
    /* -----------------------------Recibir pid----------------------------------- */
            uint32_t pid = recibir_pid(lista);
            log_info(logger,"\t El pid de la patota es: %d\n",pid);
            t_pcb* pcb = malloc(sizeof(t_pcb));
            pcb->pid = pid;
    
    /* ------------------------Cantidad de tripulantes---------------------------- */
            int cant_tripulantes = recibir_catidad_de_tripulantes(lista);
            log_info(logger," Los tripulantes son %d\n",cant_tripulantes);

    /* ------------------------Creacion de tcbs---------------------------- */
            t_list* tcbs = crear_tcbs(lista,cant_tripulantes);
            // list_iterate(tcbs, (void*) iterator_tcb);


    /* ------------------------Creacion de tareas---------------------------- */
            // t_list* tareas;
            // tareas = crear_tareas(lista);

            char* tareas_unidas = unir_tareas(lista);

			// printf("\tMe llegaron los siguientes valores:\n");
			// list_iterate(tareas, (void*) iterator_tarea);

    /* ------------------------Asignacion de segmentos---------------------------- */
    /* ---------------------validar espacio disponible-------------------------- */
        // int _tareas_bytes_ocupados = tareas_bytes_ocupados(tareas);
        int _tareas_bytes_ocupados = strlen(tareas_unidas);

        int bytes_a_guardar = _tareas_bytes_ocupados + sizeof(t_pcb) + cant_tripulantes * sizeof(t_tcb);
        loggear_linea();
        log_info(logger,"Bytes ocupados por las tareas (%d) + pcb (%d) y tcbs (%d) \n%40s son: %d bytes en total",_tareas_bytes_ocupados,sizeof(t_pcb),cant_tripulantes * sizeof(t_tcb),"",bytes_a_guardar);
        loggear_linea();
        // Se pregunta si hay espacio para TODAS los segmentos
        pthread_mutex_lock(&mutex_segmentos_libres);
        if (bytes_a_guardar >= admin_segmentacion->bytes_libres)
        {
            if (bytes_a_guardar > admin_segmentacion->bytes_libres)
            {
                log_info(logger,"No se pudo almacenar esta patota por falta de espacio");
                log_info(logger,"Bytes a ocupar: %d \n Bytes libres: %d",bytes_a_guardar,admin_segmentacion->bytes_libres);

			    free(pcb);
                list_clean_and_destroy_elements(lista, (void*) iterator_destroy);
                list_destroy(lista);
                list_clean_and_destroy_elements(tcbs, (void*) iterator_destroy_tcb);
			    list_destroy(tcbs);
                free(tareas_unidas);
                pthread_exit(pthread_self);
                break;
            }
            pthread_mutex_lock(&sem_memoria); 
            compactar();
            pthread_mutex_unlock(&sem_memoria); 
        }
        pthread_mutex_unlock(&mutex_segmentos_libres);
        
        t_list* tabla_segmentos_de_patota = (t_segmento*) list_create();
        mostrar_memoria_char();

    /* -----------------------------Tareas-------------------------------------- */
        t_segmento* segmento_tareas;
        segmento_tareas = buscar_segmento_libre(_tareas_bytes_ocupados);
        segmento_tareas->tipo = 'I';
        loggear_info_segmento(segmento_tareas,"segmento_tareas");

        guardar_en_MEMORIA_tareas(segmento_tareas,tareas_unidas);
        // mostrar_memoria();
        
        // Descomentar para testear lo que se guarda
        // char* tareas_de_MEMORIA = retornar_tareas(segmento_tareas);
        // char* tareas_solicitada = retornar_tarea_solicitada(tareas_de_MEMORIA,2);
        // log_info(logger,"Cantidad de tareas en memoria -> %d", cantidad_de_tareas(tareas_de_MEMORIA));
        // log_info(logger,"las tarea solicitada sacada de la memoria es -> %s\n",tareas_solicitada);
        // free(tareas_de_MEMORIA);


        // liberar_segmento(segmento_tareas);
    /* ------------------------------PCB---------------------------------------- */
        t_segmento* segmento_pcb;
        segmento_pcb = buscar_segmento_libre(sizeof(t_pcb));
        segmento_pcb->tipo = 'P';
        loggear_info_segmento(segmento_pcb,"segmento_pcb");
        
        pcb->tareas = segmento_tareas->inicio;
        // log_info(logger,"el pid que se guarda en memoria es %d\n",pcb->pid);
        guardar_en_MEMORIA_pcb(segmento_pcb,pcb);
        // log_info(logger,"el pid en memoria es %d\n",retornar_pid_del_pcb(segmento_pcb));
        
        free(pcb);

        // Descomentar para testear lo que se guarda

        // mostrar_memoria();
        // uint32_t pid_de_MEMORIA = retornar_pid_del_pcb(segmento_pcb);
        // uint32_t tareas_del_pcb_de_MEMORIA = retornar_inicio_de_tareas_del_pcb(segmento_pcb);
        // log_info(logger,"info de pcb retornado de memoria, \n\t pid -> %d \n\t tareas-> %d ",pid_de_MEMORIA,tareas_del_pcb_de_MEMORIA);

        // free(pid_de_MEMORIA);
        // log_info(logger,"info de pcb retornado de memoria, \n\t pid -> %d \n\t tareas-> %d ",pid_de_MEMORIA,tareas_del_pcb_de_MEMORIA);
        // free(pcb);

        list_add(tabla_segmentos_de_patota,segmento_pcb);
        // liberar_segmento(segmento_pcb);
    /* ------------------------------TCBs--------------------------------------- */
        t_list_iterator* list_iterator = list_iterator_create(tcbs);
        t_tcb* tcb;
        while(list_iterator_has_next(list_iterator))
        {
            tcb = (t_tcb*) list_iterator_next(list_iterator);
            tcb->puntero_pcb = segmento_pcb->inicio;

            t_segmento* segmento_tcb = buscar_segmento_libre(sizeof(t_tcb));
            segmento_tcb->tipo = 'T';

            guardar_en_MEMORIA_tcb(segmento_tcb,tcb);
            list_add(tabla_segmentos_de_patota,segmento_tcb);
            
            log_info(logger,"TCB-%d",list_iterator->index);
            loggear_info_segmento(segmento_tcb,"segmento_tcb");

            // Descomentar para testear lo que se guarda

            // loggear_entero_con_texto("El tid es:",retornar_tid_del_tcb(segmento_tcb));
            // log_info(logger,"va el tid del tcb-> %d",retornar_tid_del_tcb(segmento_tcb));
            // log_info(logger,"va estado del tcb-> %c",retornar_estado_del_tcb(segmento_tcb));
            // log_info(logger,"El pos x es: %d",retornar_pos_x_del_tcb(segmento_tcb));
            // liberar_segmento(segmento_tcb);
        // mostrar_memoria();
        // log_info(logger,"el puntero del pcb dentro del tcb es: %d",retornar_puntero_pid_del_tcb(segmento_tcb));
        }
        // mostrar_memoria_char();
        free(list_iterator);

    /*----------------------Fin Asignacion de segmentos--------------------------*/
    /*-----------------------Hidratacion de la patota----------------------------*/

    // Esto tendra el orden de PCB, TCB1 .. TCB N, tareas (en la posicion N de la lista)
        list_add(tabla_segmentos_de_patota,segmento_tareas);
        list_add_sorted(lista_de_patotas,tabla_segmentos_de_patota,comparador_patotas);
        // list_iterate(lista_de_patotas, (void*) iterator_patota);

        // t_list* patotita = list_get(lista_de_patotas,0);
        // t_segmento* patotita_pcb = list_get(patotita,0);
        // loggear_info_segmentos(patotita_pcb,"patotita_pcb");
        // log_info(logger, "pcb de lista de patotas %d",retornar_pid_del_pcb(patotita_pcb));
        
    /*---------------------Fin Hidratacion de la patota--------------------------*/
        // liberar_segmento(segmento_tareas);
        // liberar_segmento(segmento_pcb);
    /*-----------------------Liberacion de estructuras---------------------------*/
            list_clean_and_destroy_elements(lista, (void*) iterator_destroy);
            list_clean_and_destroy_elements(tcbs, (void*) iterator_destroy_tcb);
			list_destroy(tcbs);
            free(tareas_unidas);
            // liberar_segmento(segmento_tareas);
            // liberar_segmento(segmento_pcb);
            // liberar_segmento(segmento_tcb);
            // list_clean_and_destroy_elements(tabla_segmentos_de_patota, (void*) iterator_destroy);
			// list_destroy(tabla_segmentos_de_patota);
            // list_clean_and_destroy_elements(tareas, (void*) iterator_destroy_tarea);
			// list_destroy(tareas);
            // liberar_espacio_de_memoria(); // BORRAR ESTOOO!
            break;
		case ENVIAR_PROXIMA_TAREA:
        // DISCORDIADOR me manda un pid y un tid
            log_info(logger,"Una tarea se esta pidiendo!");
            recibir_paquete(cliente_fd,lista);

            loggear_linea();
            uint32_t pid_prox_tarea = recibir_pid(lista);
            uint32_t tid__prox_tarea = recibir_pid(lista);
            // log_info(logger,"El pid_prox_tarea de la patota es: %d",pid_prox_tarea);
            // log_info(logger,"El tid__prox_tarea de la patota es: %d\n",tid__prox_tarea);
            

        //  lista de segmentos de ese pid de las estructuras administrativas
            t_list* segmentos_patota;
            segmentos_patota = retornar_segmentos_patota(pid_prox_tarea);
            
            // Obtenemos el segmento de TAREAS de las estructuras administrativas
            t_segmento* segmento_tareas_prox_tarea;
            segmento_tareas_prox_tarea = retornar_segmento_tareas(segmentos_patota);

            // Obtenemos el segmento de TCB ESPECIFICO (TID) las estructuras administrativas
            t_segmento* segmento_tcb_prox_tarea;
            segmento_tcb_prox_tarea = retornar_segmento_tcb(segmentos_patota,tid__prox_tarea);

            // Entero que representa la proxima tarea del tripulante, se actualiza despues de devolver
            int prox_tarea = retornar_prox_inst_del_tcb(segmento_tcb_prox_tarea);

            // se calcula el tamanio de la tareas unidas
            int tamanio_tarea = segmento_tareas_prox_tarea->fin - segmento_tareas_prox_tarea->inicio;
            log_info(logger,"El tamanio es: %d",tamanio_tarea);

            // Obtenemos las TAREAS que se encuentran en MEMORIA
            char* tareas_de_segmentos_patota;
            tareas_de_segmentos_patota = retornar_tareas(segmento_tareas_prox_tarea);
            log_info(logger,"El taeritas es: %s",tareas_de_segmentos_patota);

            // Obtenemos las TAREA ESPECIFICA (la proxima) que se encuentran en MEMORIA
            char* tarea_solicitada;
            tarea_solicitada = retornar_tarea_solicitada(tareas_de_segmentos_patota,prox_tarea);
            log_info(logger,"El taerita solicitada es: %s",tarea_solicitada);


            // enviar_proxima_tarea(tarea_solicitada, cliente_fd);
            // enviar_mensaje(tarea_solicitada,cliente_fd);
            // t_paquete* paquete = crear_paquete(ENVIAR_PROXIMA_TAREA);
            // agregar_a_paquete(paquete, tarea_solicitada,strlen(tarea_solicitada));
            // enviar_paquete(paquete,cliente_fd);

            // Descomentar
            // enviar_mensaje(tarea_solicitada,cliente_fd);

            // liberar estructuras utilizadas

            free(tareas_de_segmentos_patota);
            free(tarea_solicitada);
            list_clean_and_destroy_elements(lista, (void*) iterator_destroy);
			break;
        case ACTUALIZAR_ESTADO:
        // DISCORDIADOR me manda un pid y un tid
            log_info(logger,"Se esta pidiendo una actualizacion de estado!");
            recibir_paquete(cliente_fd,lista);

            loggear_linea();
            uint32_t pid_actualizar_estado = recibir_pid(lista);
            uint32_t tid_actualizar_estado = recibir_pid(lista);
            char estado_actualizar_estado = recibir_estado(lista);
            // log_info(logger,"El pid_actualizar_estado de la patota es: %d",pid_actualizar_estado);
            // log_info(logger,"El tid_actualizar_estado de la patota es: %d\n",tid_actualizar_estado);
            

        //  lista de segmentos de ese pid de las estructuras administrativas
            t_list* segmentos_patota_actualizar_estado;
            segmentos_patota_actualizar_estado = retornar_segmentos_patota(pid_actualizar_estado);
            
            // Obtenemos el segmento de TAREAS de las estructuras administrativas
            // t_segmento* segmento_tareas_actualizar_estado;
            // segmento_tareas_actualizar_estado = retornar_segmento_tareas(segmentos_patota_actualizar_estado);

            // Obtenemos el segmento de TCB ESPECIFICO (TID) las estructuras administrativas
            t_segmento* segmento_tcb_actualizar_estado;
            segmento_tcb_actualizar_estado = retornar_segmento_tcb(segmentos_patota_actualizar_estado,tid_actualizar_estado);
            loggear_tcb(segmento_tcb_actualizar_estado);

            // Actualizar el estado del tripulante
            actualizar_en_MEMORIA_tcb_estado(segmento_tcb_actualizar_estado,estado_actualizar_estado);
            loggear_linea();
            log_info(logger,"Lo proximo se esta consultado desde memoria");
            log_info(logger,"El nuevo estado es: %c",estado_actualizar_estado);

            loggear_tcb(segmento_tcb_actualizar_estado);
            // list_iterate(lista_de_patotas, (void*) iterator_patota);
            // enviar_proxima_tarea(tarea_solicitada, cliente_fd);


            // liberar estructuras utilizadas

            list_clean_and_destroy_elements(lista, (void*) iterator_destroy);
            break;
        case EXPULSAR_TRIPULANTE:
            log_info(logger,"Un tripulante se esta por despedir!");
            recibir_paquete(cliente_fd,lista);

            loggear_linea();
            uint32_t pid_expulsar_tripulante = recibir_pid(lista);
            uint32_t tid_expulsar_tripulante = recibir_pid(lista);

            // lista de segmentos de ese pid de las estructuras administrativas
            t_list* segmentos_patota_expulsar_tripulante;
            segmentos_patota_expulsar_tripulante = retornar_segmentos_patota(pid_expulsar_tripulante);
            // log_info(logger,"\ttesteo");
            // list_iterate(lista_de_patotas, (void*) iterator_patota);

            // Obtenemos el segmento de TCB de las estructuras administrativas
            t_segmento* segmento_tcb_explulsar_tripulante;
            segmento_tcb_explulsar_tripulante = retornar_segmento_tcb(segmentos_patota_expulsar_tripulante,tid_expulsar_tripulante);
            log_info(logger,"De la patota %d,\n %39s se nos fue el tripulante : %d",pid_expulsar_tripulante,"",tid_expulsar_tripulante);
            liberar_segmento(segmento_tcb_explulsar_tripulante);

            break;
        case COMPACTAR:

            compactar();
            // t_list* segmentos_ocupados = lista_de_segmentos_ocupados();
            
            // compactar(segmentos_ocupados);
            
            // list_destroy(segmentos_ocupados);
            break;
        case RECIBIR_LA_UBICACION_DEL_TRIPULANTE:
            log_info(logger,"Se movio un tripulante, parate un tantito que actualizo la info!");
            recibir_paquete(cliente_fd,lista);
            uint32_t pid_ubicacion = recibir_pid(lista);
            uint32_t tid_ubicacion = recibir_pid(lista);

            uint32_t nueva_pos_x = recibir_pid(lista);
            uint32_t nueva_pos_y = recibir_pid(lista);

            // lista de segmentos de ese pid de las estructuras administrativas
            t_list* segmentos_patota_ubicacion;
            segmentos_patota_ubicacion = retornar_segmentos_patota(pid_ubicacion);

            t_segmento* segmento_tcb_ubicacion;
            segmento_tcb_ubicacion = retornar_segmento_tcb(segmentos_patota_ubicacion,tid_ubicacion);

            loggear_tcb(segmento_tcb_ubicacion);
            actualizar_en_MEMORIA_tcb_posiciones(segmento_tcb_ubicacion,nueva_pos_x,nueva_pos_y);
            
            loggear_tcb(segmento_tcb_ubicacion);
            break;
		case FINALIZACION:
			printf("\tSe acabo todo, nos vemos!\n");
			// int size;
			// recibir_mensaje(cliente_fd,logger,&size);
            // free(MEMORIA); 
            // terminar_miramhq(logger,config);

			log_info(logger, "Nos estamos viendo. Abrazo procesin");
            return EXIT_SUCCESS;
			// break;
		case -1:
			log_error(logger, "el cliente se desconecto. Terminando servidor");
			return EXIT_FAILURE;
		default:
			log_warning(logger, "Operacion desconocida. No quieras meter la pata");
			break;
		}

        // int size;
        // log_info(logger, (char*) argv[0]);
        // recibir_mensaje(cliente_fd,logger,&size);
        // log_info(logger, (char*) argv[1]);

        list_destroy(lista);
        // loggear_linea();
        // list_iterate(admin_segmentacion->segmentos_libres, (void*) iterator_segmento);
        // loggear_linea();
        // imprimir_patotas_presentes();
        loggear_linea();
        // liberar_conexion(cliente_fd);
        pthread_exit(pthread_self);
}


/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */

/* ----------------------Administrar Memoria-------------------------- */

void reservar_espacio_de_memoria()
{
    SMOBJ_SIZE = config_get_int_value(config,"TAMANIO_MEMORIA");
    MEMORIA = (void*) malloc (SMOBJ_SIZE);

};

void liberar_espacio_de_memoria()
{
    // list_clean_and_destroy_elements(admin_segmentacion->segmentos_libres, (void*) iterator_destroy);
    list_iterate(admin_segmentacion->segmentos_libres,iterator_segmentos_free);
    // free(list_remove(admin_segmentacion->segmentos_libres,2));
    list_destroy(admin_segmentacion->segmentos_libres);
    free(admin_segmentacion);

    free(MEMORIA);
};

t_segmento* ultimo_segmento_libre()
{
    return (t_segmento*) list_get(admin_segmentacion->segmentos_libres,list_size(admin_segmentacion->segmentos_libres)-1);
}

t_segmento* ultimo_segmento_libre_compactable()
{
    // consolidar_segmentos_libres_se_es_posbile();
    t_segmento* ultimo_segmento;
    if (list_size(admin_segmentacion->segmentos_libres)>1 && ultimo_segmento_libre()->fin == SMOBJ_SIZE)
    {
        // ultimo_segmento = (t_segmento*) list_get(admin_segmentacion->segmentos_libres,list_size(admin_segmentacion->segmentos_libres)-1);
        ultimo_segmento = (t_segmento*) list_get(admin_segmentacion->segmentos_libres,list_size(admin_segmentacion->segmentos_libres)-2);
    }else
    {
        ultimo_segmento = ultimo_segmento_libre();
    }

    return ultimo_segmento;
}

void consolidar_segmentos_libres_se_es_posbile()
{
    t_segmento* anteultimo, *ultimo;
    if (list_size(admin_segmentacion->segmentos_libres)>1)
    {
        anteultimo = list_get(admin_segmentacion->segmentos_libres,list_size(admin_segmentacion->segmentos_libres)-2);
        int diferencia_anteultimo = anteultimo->fin - anteultimo->inicio +1;
        ultimo = ultimo_segmento_libre();
        // log_info(logger,"\tanteultimo byte: %d",anteultimo->fin);
        // log_info(logger,"\tultimo byte: %d",ultimo->inicio);

        if (anteultimo->fin+1 == ultimo->inicio)
        {
            ultimo->inicio -= diferencia_anteultimo;
            anteultimo = list_remove(admin_segmentacion->segmentos_libres,list_size(admin_segmentacion->segmentos_libres)-2);
            free(anteultimo);
        }

        
    }

    // if (list_size(admin_segmentacion->segmentos_libres) == 1)
    // {
    //     anteultimo = list_get(admin_segmentacion->segmentos_libres,list_size(admin_segmentacion->segmentos_libres)-1);
    //     ultimo = ultimo_segmento_libre();
    //     if (anteultimo->fin == ultimo->inicio)
    //     {   
    //         anteultimo = list_remove(admin_segmentacion->segmentos_libres,list_size(admin_segmentacion->segmentos_libres)-2);
    //         free(anteultimo);
    //     }
    // }
    
}

t_list* segmentos_ocupados_afectados(t_list* lista_ocupados)
{
    t_list* lista_ocupados_filtrados;

    lista_ocupados_filtrados = list_filter(lista_ocupados, condicion_segmento_afectado);
    // list_clean(lista_ocupados);
    // list_destroy(lista_ocupados);
    return lista_ocupados_filtrados;
}


/* ------------------------------tarea-------------------------------- */

int tareas_bytes_ocupados(t_list* tareas)
{
    t_list_iterator* list_iterator = list_iterator_create(tareas);
    int bytes_ocupados = 0;

    while(list_iterator_has_next(list_iterator))
    {
        t_tarea* tarea = (t_tarea*) list_iterator_next(list_iterator);
        
        bytes_ocupados += sizeof(t_tarea);
        bytes_ocupados += strlen(tarea->accion);
    }

    list_iterator_destroy(list_iterator);

    return bytes_ocupados;
}

// int cantidad_de_tareas(char* tareas)
// {
    // char** lista_tareas = string_split(tareas,"-");
//     int cant_tareas = list_size(lista_tareas);
//     loggear_entero(cant_tareas);
//     string_iterate_lines(lista_tareas, iterator_lines_free);
//     free(lista_tareas);
//     return cant_tareas;
// }
/* --------------------Fin Administrar Memoria------------------------ */

/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */

/* --------------------------Segmentacion----------------------------- */

void iniciar_segmentacion()
{
    admin_segmentacion = malloc(sizeof(administrador_de_segmentacion));
    t_list* segmentos_libre = list_create();
    // admin_segmentacion->segmentos_libres = list_create();
    t_segmento* segmento = malloc(sizeof(t_segmento));

    segmento->inicio = (uint32_t) 0;
    segmento->fin = (uint32_t) config_get_int_value(config,"TAMANIO_MEMORIA");
    segmento->se_encuentra = 0;
    segmento->tipo = 'L';

    list_add(segmentos_libre,segmento);
    // list_add(admin_segmentacion->segmentos_libres,segmento);
    admin_segmentacion->bytes_libres = (uint32_t) config_get_int_value(config,"TAMANIO_MEMORIA");
    admin_segmentacion->segmentos_libres = segmentos_libre;

    limpiar_memoria();

    lista_de_patotas = list_create();
};


void asignar_segmento(uint32_t bytes_ocupados)
{
    t_segmento* segmento = buscar_segmento_libre(bytes_ocupados);


    free(segmento);
};

t_segmento* buscar_segmento_libre(uint32_t bytes_ocupados)
{
    char* criterio = config_get_string_value(config,"CRITERIO");
    t_segmento* segmento_libre_buscado;
    // log_info(logger,"el criterio es: %s",criterio);
    if ( strcmp(criterio,"FIRSTFIT") == 0)
    {
        segmento_libre_buscado = buscar_segmento_libre_first_fit(bytes_ocupados);
    }else
    {
        segmento_libre_buscado = buscar_segmento_libre_best_fit(bytes_ocupados);
    }

    // switch (criterio)
    // {
    // case "FIRSTFIT":
    //     /* code */
    //     break;
    
    // default:
    //     break;
    // }

    return segmento_libre_buscado;
}




t_segmento* buscar_segmento_libre_first_fit(uint32_t bytes_ocupados)
{
    pthread_mutex_lock(&mutex_segmentos_libres);

    t_list_iterator* list_iterator = list_iterator_create(admin_segmentacion->segmentos_libres);
    bool encontrado = false;

    t_segmento* segmento_libre,* segmento_ocupado;


    while(!encontrado && list_iterator_has_next(list_iterator))
    {
        segmento_libre = (t_segmento*) list_iterator_next(list_iterator);
        int diferencia = (segmento_libre->fin - segmento_libre->inicio);

        if (bytes_ocupados <= diferencia)
        {
        admin_segmentacion->bytes_libres -= bytes_ocupados;

            if (bytes_ocupados < diferencia)
            {
                segmento_ocupado = malloc(sizeof(t_segmento));
                // segmento_ocupado->se_encuentra = 1;
                segmento_ocupado->inicio = (uint32_t) segmento_libre->inicio;
                segmento_ocupado->fin = (uint32_t) segmento_libre->inicio + (uint32_t) bytes_ocupados - 1;

                segmento_libre->inicio += bytes_ocupados;
                
                
                // list_replace(admin_segmentacion->segmentos_libres,list_iterator->index,segmento_ocupado);
                // t_segmento* segmento_a_borrar = list_replace(admin_segmentacion->segmentos_libres,list_iterator->index,segmento_ocupado);
                // list_replace_and_destroy_element(admin_segmentacion->segmentos_libres,list_iterator->index,segmento_ocupado,iterator_destroy);
                // free(segmento_a_borrar);
                // free(segmento_ocupado);
            }else
            {
                segmento_ocupado = list_remove(admin_segmentacion->segmentos_libres,list_iterator->index);
                // list_remove_and_destroy_element(admin_segmentacion->segmentos_libres,list_iterator->index,iterator_segmentos_free);
            }
            // segmento->fin = segmento->inicio + bytes_ocupados; 
            encontrado = true;
        }
        
        // free(segmento);
    }

    // string_iterate_lines(list_iterator, iterator_segmentos_free);
    free(list_iterator);

    if (encontrado)
    {
        segmento_ocupado->se_encuentra = 1;
        pthread_mutex_unlock(&mutex_segmentos_libres);
        return segmento_ocupado;
        
    }
    else{
        log_info(logger,"No hay espacio para colocar esta cantidad de bytes, te recomendaria compactar");
        pthread_mutex_unlock(&mutex_segmentos_libres);
        return NULL;
    }
}

t_segmento* buscar_segmento_libre_best_fit(uint32_t bytes_ocupados)
{
    pthread_mutex_lock(&mutex_segmentos_libres);
    list_sort(admin_segmentacion->segmentos_libres,orden_lista_admin_segmentacion_best_free);
    t_list_iterator* list_iterator = list_iterator_create(admin_segmentacion->segmentos_libres);
    bool encontrado = false;
    
    t_segmento* segmento_libre;
    t_segmento* segmento_ocupado = malloc(sizeof(t_segmento));
    while(!encontrado && list_iterator_has_next(list_iterator))
    {
        segmento_libre = (t_segmento*) list_iterator_next(list_iterator);
        uint32_t diferencia = (segmento_libre->fin - segmento_libre->inicio)+1;
    
        if(bytes_ocupados < diferencia)
        {
            admin_segmentacion->bytes_libres -= bytes_ocupados;

            segmento_ocupado->inicio = (uint32_t) segmento_libre->inicio;
            segmento_ocupado->fin = (uint32_t) segmento_libre->inicio + (uint32_t) bytes_ocupados - (uint32_t) 1;
            segmento_libre->inicio += bytes_ocupados;
            
            encontrado = true;
        }else
        {
            segmento_ocupado = list_remove(admin_segmentacion->segmentos_libres,list_iterator->index);
        }
    }
    free(list_iterator);
    
    list_sort(admin_segmentacion->segmentos_libres,orden_lista_admin_segmentacion);
    
    // list_iterate(admin_segmentacion->segmentos_libres, (void*) iterator_segmento);

    pthread_mutex_unlock(&mutex_segmentos_libres);
    segmento_ocupado->se_encuentra = 1;

    return segmento_ocupado;
}

t_segmento* buscar_segmento_libre_best_fit_original(uint32_t bytes_ocupados)
{
    t_list_iterator* list_iterator = list_iterator_create(admin_segmentacion->segmentos_libres);

    t_segmento* segmento_libre;
    uint32_t menor_distancia = SMOBJ_SIZE;
    // t_segmento* segmento_elegido;
    t_segmento* segmento_ocupado = malloc(sizeof(t_segmento));

    while(list_iterator_has_next(list_iterator))
    {
        segmento_libre = (t_segmento*) list_iterator_next(list_iterator);
        uint32_t diferencia = (segmento_libre->fin - segmento_libre->inicio)+1;
    
        if(bytes_ocupados <= diferencia && diferencia <= menor_distancia)
        {
            segmento_ocupado->inicio = (uint32_t) segmento_libre->inicio;
            segmento_ocupado->fin = (uint32_t) segmento_libre->inicio + (uint32_t) bytes_ocupados - (uint32_t) 1;

            menor_distancia = diferencia;
            
        }
    }
    segmento_libre->inicio += bytes_ocupados;
    free(list_iterator);

    log_info(logger,"El inicio del \"%s\" es %d, el fin %d y se encuentra en ram: %d \n","tareas", segmento_ocupado->inicio, segmento_ocupado->fin, segmento_ocupado->se_encuentra);



    return segmento_ocupado;
}


void guardar_en_MEMORIA_tcb(t_segmento* segmento_a_ocupar,t_tcb* tcb)
{
    pthread_mutex_lock(&sem_memoria);    
        int desplazamiento = segmento_a_ocupar->inicio;
    // segmento_a_ocupar->tipo = 't';
    
    memcpy(MEMORIA + desplazamiento, (void*) (&tcb->tid), sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(MEMORIA + desplazamiento, (void*) (&tcb->estado), sizeof(char));
    desplazamiento += sizeof(char);
    memcpy(MEMORIA + desplazamiento, (void*) (&tcb->posicion_x), sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(MEMORIA + desplazamiento, (void*) (&tcb->posicion_y), sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(MEMORIA + desplazamiento, (void*) (&tcb->proxima_instruccion), sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(MEMORIA + desplazamiento, (void*) (&tcb->puntero_pcb), sizeof(uint32_t));
    pthread_mutex_unlock(&sem_memoria); 
    
    desplazamiento += sizeof(uint32_t)-1;

    if (desplazamiento != segmento_a_ocupar->fin)
    {
        log_info(logger,"Un tcb se cargo mal a memoria");
    }
}

void actualizar_en_MEMORIA_tcb_prox_tarea(t_segmento* segmento_a_modificar)
{
        pthread_mutex_lock(&sem_memoria); 
        int desplazamiento_proxima_tarea = segmento_a_modificar->inicio;
        desplazamiento_proxima_tarea += sizeof(uint32_t)+sizeof(char)+sizeof(uint32_t)+sizeof(uint32_t);
        // uint32_t* prox_tarea_aux = malloc(sizeof(uint32_t));
        char* MEMORIA_prox_tarea = MEMORIA;
        // strcpy(prox_tarea_aux, MEMORIA_prox_tarea[desplazamiento_proxima_tarea]);
        uint32_t prox_tarea_aux = MEMORIA_prox_tarea[desplazamiento_proxima_tarea];
        prox_tarea_aux += 1;
        // strcpy(prox_tarea_aux,MEMORIA_prox_tarea[desplazamiento_proxima_tarea]+1);
        // strcpy(prox_tarea_aux,prox_tarea_aux+1);
        // memcpy(prox_tarea_aux, (void*) (&tcb->puntero_pcb), sizeof(uint32_t));
        memcpy(MEMORIA + desplazamiento_proxima_tarea, (&prox_tarea_aux), sizeof(uint32_t));
        pthread_mutex_unlock(&sem_memoria); 
        // free(prox_tarea_aux);
}

void actualizar_en_MEMORIA_tcb_estado(t_segmento* segmento_a_modificar, char estado)
{
        pthread_mutex_lock(&sem_memoria); 
        int desplazamiento_estado = segmento_a_modificar->inicio;
        desplazamiento_estado += sizeof(uint32_t);

        memcpy(MEMORIA + desplazamiento_estado, (&estado), sizeof(char));
        pthread_mutex_unlock(&sem_memoria); 
}

void actualizar_en_MEMORIA_tcb_posiciones(t_segmento* segmento_a_modificar, uint32_t pos_x, uint32_t pos_y)
{
        pthread_mutex_lock(&sem_memoria); 
        int desplazamiento_posiciones = segmento_a_modificar->inicio;
        desplazamiento_posiciones += sizeof(uint32_t) +sizeof(char);

        memcpy(MEMORIA + desplazamiento_posiciones, (&pos_x), sizeof(uint32_t));
        desplazamiento_posiciones += sizeof(uint32_t);
        memcpy(MEMORIA + desplazamiento_posiciones, (&pos_y), sizeof(uint32_t));
        pthread_mutex_unlock(&sem_memoria); 
}


void guardar_en_MEMORIA_pcb(t_segmento* segmento_a_ocupar,t_pcb* pcb)
{
        int desplazamiento = segmento_a_ocupar->inicio;
    // segmento_a_ocupar->tipo = 'p';

    // loggear_entero(pcb->pid);
    // loggear_entero(pcb->tareas);

    // uint32_t* pid = malloc(sizeof(uint32_t));
    // pid = pcb->pid;
    // uint32_t* tareas = malloc(sizeof(uint32_t));
    // tareas = pcb->tareas;
    // char* pid = malloc(strlen(string_itoa(pcb->pid)+1));
    // pid = (char*) string_itoa(pcb->pid);
    // log_info(logger,"el pid es %s", pid);

    // char* tareas = malloc(strlen(string_itoa(pcb->tareas)+1));
    // tareas = (char*) string_itoa(pcb->tareas);
    // log_info(logger,"las tareas estan en es %s", tareas);
    // loggear_info_segmentos(segmento_a_ocupar,"Segmento a ocupar");
    pthread_mutex_lock(&sem_memoria); 
    memcpy(MEMORIA + desplazamiento, (&pcb->pid), sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(MEMORIA + desplazamiento, (&pcb->tareas), sizeof(uint32_t));
    pthread_mutex_unlock(&sem_memoria); 
    desplazamiento += sizeof(uint32_t)-1;
    // loggear_entero(desplazamiento);
    if (desplazamiento != segmento_a_ocupar->fin)
    {
        log_info(logger,"Un pcb se cargo mal a memoria");
    }
    // loggear_entero(segmento_a_ocupar->fin);

}

void guardar_en_MEMORIA_tareas(t_segmento* segmento_a_ocupar,char* tareas_unidas)
{
    int desplazamiento = segmento_a_ocupar->inicio;
    // segmento_a_ocupar->tipo = 'i';

    pthread_mutex_lock(&sem_memoria); 
    memcpy(MEMORIA + desplazamiento, (void*) (tareas_unidas), strlen(tareas_unidas));
    pthread_mutex_unlock(&sem_memoria); 
    
    desplazamiento += strlen(tareas_unidas)-1;
    if (desplazamiento != segmento_a_ocupar->fin)
    {
        log_info(logger,"Las tareas se cargaron mal a memoria");
    }
}




void liberar_segmento(t_segmento* segmento)
{
    pthread_mutex_lock(&mutex_segmentos_libres);

    admin_segmentacion->bytes_libres += segmento->fin - segmento->inicio;
    
    for (int i = segmento->inicio; i < segmento->fin; i++)
    {
        pthread_mutex_lock(&sem_memoria); 
        memcpy(MEMORIA+i, (void*) "", sizeof(char));
        pthread_mutex_unlock(&sem_memoria); 
    }
    
    segmento->se_encuentra = 0;
    segmento->tipo = 'L';
    list_add(admin_segmentacion->segmentos_libres, segmento);
    
    // list_add_sorted(admin_segmentacion->segmentos_libres, segmento,orden_lista_admin_segmentacion);
    list_sort(admin_segmentacion->segmentos_libres,orden_lista_admin_segmentacion);
    pthread_mutex_unlock(&mutex_segmentos_libres);
}

void compactar()
{
    pthread_mutex_lock(&mutex_segmentos_libres);
    t_list* lista_ocupada = lista_de_segmentos_ocupados();
            
    // loggear_linea();
    log_info(logger,"\tInfo previa");
    loggear_linea();
    log_info(logger,"\tSegmentos Ocupados");
    list_iterate(lista_ocupada, (void*) iterator_segmento);
    loggear_linea();
    log_info(logger,"\tSegmentos libres");
    list_iterate(admin_segmentacion->segmentos_libres, (void*) iterator_segmento);
    loggear_linea();
    log_info(logger,"\tMemoria completa: %d",SMOBJ_SIZE);
    loggear_linea();

    // t_list* segmentos_afectados,* segmentos_transformados;
    t_list* segmentos_afectados;
    t_list* * segmentos_transformados;


    // loggear_linea();
    // log_info(logger,"\ttesteo previo a mappear");
    // list_iterate(lista_de_patotas, (void*) iterator_patota);
    // loggear_linea();

    log_info(logger,"\tComienza la Compactacion");
    while (list_size(admin_segmentacion->segmentos_libres)>1 && ultimo_segmento_libre_compactable()->fin != SMOBJ_SIZE)
    {
        consolidar_segmentos_libres_se_es_posbile();
        // ult_segmento_libre = ultimo_segmento_libre_compactable();
        segmentos_afectados = segmentos_ocupados_afectados(lista_ocupada);
        loggear_linea();
        log_info(logger,"\tSegmentos ocupados desplazados");
        list_iterate(segmentos_afectados, (void*) iterator_segmento);
        // ultimo_segmento_ocupado = list_get(segmentos_afectados,0);
        segmentos_transformados = list_map(segmentos_afectados, transformacion_segmento_afectado);
        // list_iterate(segmentos_afectados, (void*) iterator_segmento);
        list_destroy(segmentos_afectados);
        list_destroy(segmentos_transformados);
    }
    list_destroy(lista_ocupada);
    // loggear_linea();
    // log_info(logger,"\tSegmentos ocupados desplazados");
    // list_iterate(segmentos_afectados, (void*) iterator_segmento);

    // list_clean_and_destroy_elements(segmentos_transformados, (void*) iterator_segmentos_free);
    // list_clean(segmentos_transformados);
    // list_clean_and_destroy_elements(segmentos_afectados, (void*) iterator_segmentos_free);
    // list_clean(segmentos_afectados);


    // loggear_linea();
    // ult_segmento_libre = ultimo_segmento_libre_compactable();
    // segmentos_afectados = segmentos_ocupados_afectados(lista_ocupada);
    // list_map(segmentos_afectados, transformacion_segmento_afectado);
    // list_iterate(segmentos_afectados, (void*) iterator_segmento);
    // loggear_linea();
    // list_clean_and_destroy_elements(segmentos_transformados, (void*) iterator_destroy_tcb);
    // list_clean_and_destroy_elements(segmentos_transformados, (void*) iterator_segmentos_free);
    // list_clean(segmentos_transformados);
    // list_destroy(segmentos_transformados);
    // list_clean_and_destroy_elements(segmentos_afectados, (void*) iterator_segmentos_free);
    // list_destroy(segmentos_afectados);
    // list_add_all(lista_de_patotas,lista_de_patotas);
    loggear_linea();
    log_info(logger,"\tFinalizo la Compactacion");
    list_iterate(admin_segmentacion->segmentos_libres, (void*) iterator_segmento);
    
    log_info(logger,"\ttesteo");
    list_iterate(lista_de_patotas, (void*) iterator_patota);
    
    imprimir_patotas_presentes();
    
    // t_segmento* nuevo = malloc(sizeof(t_segmento));
    // nuevo->inicio = 51;
    // nuevo->fin = 71;
    // int despl = nuevo->fin - nuevo->inicio;
    // int nro;
    // nro = retornar_tid_del_tcb(nuevo);
    // log_info(logger,"tid %d",nro);

    // nuevo->inicio = 43;
    // nuevo->fin = 50;
    // int despl = nuevo->fin - nuevo->inicio;
    // t_pcb* tarea_nueva = malloc(despl);
    // int* nro;
    // nro = retornar_pid_del_pcb(nuevo);
    // log_info(logger,"pcb %d",nro);

    pthread_mutex_unlock(&mutex_segmentos_libres);

}

void colocar_ultimo_segmento_libre_al_final()
{
    if (ultimo_segmento_libre()->fin != SMOBJ_SIZE)
    {
        int diferencia = ultimo_segmento_libre()->fin - ultimo_segmento_libre()->inicio;
        ultimo_segmento_libre()->fin = SMOBJ_SIZE;
        ultimo_segmento_libre()->inicio = SMOBJ_SIZE - diferencia;
        // desplazar_memoria(1,diferencia);
    }

}

t_list* lista_de_segmentos_ocupados()
{
    t_list* lista_ocupados = list_create();
    t_list* lista_patota_filtrados, *lista_patota;
    t_segmento* pcb, *tcb, *tareas;
    t_list_iterator* list_iterator_patotas = list_iterator_create(lista_de_patotas);
    while(list_iterator_has_next(list_iterator_patotas))
    {
        lista_patota = (t_segmento*) list_iterator_next(list_iterator_patotas);
        lista_patota_filtrados = (t_segmento*) list_filter(lista_patota,condicion_segmento_presente_en_memoria);
        list_add_all(lista_ocupados,lista_patota_filtrados);
        // list_clean(lista_patota);

    loggear_linea();
    // log_info(logger,"caca de adentro");
    // list_iterate(lista_patota_filtrados, (void*) iterator_segmento);
    // list_iterate(pcb_dudoso, (void*) iterator_pcb);

    // loggear_linea();
    // log_info(logger,"aca muere alguien");
    
    // log_info(logger,"info del primer segmento");
    // t_segmento* segment_pcb = list_get(lista_patota_filtrados,0);
    // uint32_t* pid1 = retornar_pid_del_pcb(segment_pcb);
    // // log_info(logger,"info del primer segmento %d",pcb1->inicio);
    // pid1 = 5;
    // t_pcb* pcb1 = malloc(sizeof(t_pcb));
    // pcb1->pid = pid1;
    // guardar_en_MEMO RIA_pcb(segment_pcb,pcb1);

    
    // loggear_linea();
    // log_info(logger,"caca de adentro");
    // list_iterate(lista_patota, (void*) iterator_patota);
    // loggear_linea();

        list_clean(lista_patota_filtrados);
        list_destroy(lista_patota_filtrados);
        // list_clean_and_destroy_elements(lista_patota_filtrados, (void*) iterator_destroy);
    }
    // loggear_linea();
    // log_info(logger,"caca");
    // list_iterate(lista_de_patotas, (void*) iterator_patota);
    // loggear_linea();


    // list_clean_and_destroy_elements(lista_patota_filtrados, (void*) iterator_destroy);
    // free(lista_patota_filtrados);
    // list_destroy(lista_patota_filtrados);
    // list_clean_and_destroy_elements(lista_patota, (void*) iterator_destroy_tcb);
    // free(lista_patota);

    // list_destroy(lista_patota);
    free(list_iterator_patotas);

    list_sort(lista_ocupados, orden_lista_segmentos);
    // list_iterate(lista_ocupados, (void*) iterator_segmento);

    return lista_ocupados;
}

void desplazar_memoria(int nro,int diferencia)
{
    t_segmento* ultimo_segmento_libre;
    ultimo_segmento_libre = (t_segmento*) list_get(admin_segmentacion->segmentos_libres, list_size(admin_segmentacion->segmentos_libres)-nro);
    log_info(logger,"\tEl primer byte antes es: %d",ultimo_segmento_libre->inicio);
    log_info(logger,"\tEl ultimo byte antes es: %d",ultimo_segmento_libre->fin);

    ultimo_segmento_libre->fin += diferencia; 
    ultimo_segmento_libre->inicio += diferencia;

    // if (nro==1)
    // {
    //     ultimo_segmento_libre->fin = SMOBJ_SIZE;
    // }

    log_info(logger,"\tEl primer byte despues es: %d",ultimo_segmento_libre->inicio);
    log_info(logger,"\tEl ultimo byte despues es: %d",ultimo_segmento_libre->fin);
    


}


/* ------------------------Fin Segmentacion--------------------------- */

/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */

/* ------------------------------Tarea-------------------------------- */

void mostrar_memoria_char()
{
    char* memory = (char*) MEMORIA;
    // memory[350] = '\0';
    // log_info(logger,"Memoriaaa: \n%s",(char*) memory);
    int tamanio = (uint32_t) config_get_int_value(config,"TAMANIO_MEMORIA");

    printf("se imprimira la memoria\n\n");
    for (int i = 0; i < tamanio; i++)
    {
        printf("%c",memory[i]);
    }
    printf("\n\nse imprimio la memoria\n");
}

void mostrar_memoria_entero()
{
    uint32_t* memory = (uint32_t*) MEMORIA;
    // memory[350] = '\0';
    // log_info(logger,"Memoriaaa: \n%s",(uint32_t*) memory);
    int tamanio = (uint32_t) config_get_int_value(config,"TAMANIO_MEMORIA");

    printf("se imprimira la memoria\n\n");
    for (int i = 0; i < tamanio; i++)
    {
        printf("%d",(uint32_t) memory[i]);
    }
    printf("\n\n se imprimio la memoria\n");
}

void limpiar_memoria()
{
    char* memory = (char*) MEMORIA;
    // memory[350] = '\0';
    // log_info(logger,"Memoriaaa: \n%s",(char*) memory);
    int tamanio = (uint32_t) config_get_int_value(config,"TAMANIO_MEMORIA");

    printf("se imprimira la memoria\n\n");
    for (int i = 0; i < tamanio; i++)
    {   
        pthread_mutex_lock(&sem_memoria); 
        memcpy(MEMORIA + i,(void*)"",sizeof(char));
        pthread_mutex_unlock(&sem_memoria); 
        // memory[i] = (char) "";
    }
    printf("\n\nse imprimio la memoria\n");
}

char* retornar_tareas(t_segmento* segmento_tareas)
{
    const int tamanio_segmento = segmento_tareas->fin - segmento_tareas->inicio;

    char* tareas_en_MEMORIA = (char*) malloc(tamanio_segmento+1);
    int desplazamiento = segmento_tareas->inicio;

    // mutex por bytes a utilizar - no recuerdo el nombre
    pthread_mutex_lock(&sem_memoria); 
    memcpy(tareas_en_MEMORIA, MEMORIA+desplazamiento, tamanio_segmento);
    pthread_mutex_unlock(&sem_memoria); 
    // mutex por bytes a utilizar - no recuerdo el nombre
    
    tareas_en_MEMORIA[segmento_tareas->fin] = '\0';
    // log_info(logger,"%s",(char*) MEMORIA);
    // mostrar_memoria_char();
    // log_info(logger,"las tareas_en_MEMORIA retornadas dentro de la funcion son: %s", tareas_en_MEMORIA);

    return tareas_en_MEMORIA;
}

char* retornar_tarea_solicitada(char* tareas_en_MEMORIA,int nro_de_tarea)
{
    int cant_tareas = cantidad_de_tareas(tareas_en_MEMORIA);
    if (nro_de_tarea == cant_tareas)
    {
        char* ultima_tarea = malloc(20);
        strcpy(ultima_tarea,"NULL");
        return ultima_tarea;  
    } 

    char** tarea_subs = string_split(tareas_en_MEMORIA,"-");
    log_info(logger,"la tarea a buscar -> %s",tarea_subs[nro_de_tarea]);
    // log_info(logger,"esto es asi %s",tarea_subs[3]);
    // log_info(logger,"Tamnio de la tarea %d",strlen(tarea_subs[2]));
    char* tarea_solicitada = malloc(strlen(tarea_subs[nro_de_tarea])+1);
    // char* tarea_solicitada = malloc(15);
    strcpy(tarea_solicitada,tarea_subs[nro_de_tarea]);
    // log_info(logger,"las tareas_solicitada dentro de la funcion es: %s", tarea_solicitada);
    string_iterate_lines(tarea_subs, iterator_lines_free);
    free(tarea_subs);

    return tarea_solicitada;
}

char* removeDigits(char* input)
{
    int j = 1;
    int i = 0;
    while (input[i] >= '0' && input[i] <= '9')
    {
        // printf("texto %c: \n",input[i]);

        input = input+1;
        
    }
    


    // for (int i = 0; input[i]; i++)
    // {
    //     //5
    //     if (input[i] >= '0' && input[i] <= '9')
    //     {
    //         input[i] = input[j];
    //         i--;
    //         // j++;
    //     }
    // }

    //6
    // input[j] = '\0';
    return input;
}
/* ----------------------------Fin Tarea------------------------------ */
/* ------------------------------Tcb---------------------------------- */

t_pcb* retornar_pcb(t_segmento* segmento_pcb)
{
    char* memoria = MEMORIA;
    t_pcb* pcb;
    pcb = memoria[segmento_pcb->inicio];
    return pcb;
}

uint32_t retornar_pid_del_pcb(t_segmento* segmento_pcb)
{
    char* pcb = MEMORIA;
    uint32_t pid;
    pid = pcb[segmento_pcb->inicio];
    return pid;
}

uint32_t retornar_inicio_de_tareas_del_pcb(t_segmento* segmento_pcb)
{
    char* pcb = MEMORIA;
    uint32_t inicio_tarea;
    // uint32_t inicio_tarea = malloc(sizeof(uint32_t));
    inicio_tarea = pcb[segmento_pcb->inicio+sizeof(uint32_t)];
    return inicio_tarea;
}

t_tcb* retornar_tcb(t_segmento* segmento_tcb)
{
    char* memoria = MEMORIA;
    t_tcb* tcb = (t_tcb*) memoria[segmento_tcb->inicio];
    return tcb;
}

int retornar_tid_del_tcb(t_segmento* segmento_tcb)
{
    char* tcb = MEMORIA;
    int tid = tcb[segmento_tcb->inicio];
    return tid;
}

char retornar_estado_del_tcb(t_segmento* segmento_tcb)
{
    char* tcb = MEMORIA;
    char estado = tcb[segmento_tcb->inicio+4];
    return estado;
}

int retornar_pos_x_del_tcb(t_segmento* segmento_tcb)
{
    char* tcb = MEMORIA;
    int pos_x = tcb[segmento_tcb->inicio+5];
    return pos_x;
}

int retornar_pos_y_del_tcb(t_segmento* segmento_tcb)
{
    char* tcb = MEMORIA;
    int pos_y = tcb[segmento_tcb->inicio+9];
    return pos_y;
}

int retornar_prox_inst_del_tcb(t_segmento* segmento_tcb)
{
    char* tcb = MEMORIA;
    int prox_inst = tcb[segmento_tcb->inicio+13];
    actualizar_en_MEMORIA_tcb_prox_tarea(segmento_tcb);
    return prox_inst;
}

uint32_t retornar_puntero_pid_del_tcb(t_segmento* segmento_tcb)
{
    char* tcb = MEMORIA;
    int desplazamiento_puntero = segmento_tcb->inicio + 4*sizeof(uint32_t) + sizeof(char);
    uint32_t puntero_pcb = tcb[desplazamiento_puntero];
    return puntero_pcb;
}

t_list* retornar_segmentos_patota(uint32_t pid)
{
    // t_list* segmentos_patota = malloc(sizeof(t_segmento));
    t_list* segmentos_patota;
    // t_segmento* patota_segmento_tcb = malloc(sizeof(t_tcb));

    for (int i = 0; i < list_size(lista_de_patotas); i++)
            {
                segmentos_patota = list_get(lista_de_patotas,i);
                t_segmento* patota_segmento_pcb = (t_segmento*) list_get(segmentos_patota,0);
                uint32_t patota_pcb_pid = retornar_pid_del_pcb(patota_segmento_pcb);
                
                if (patota_pcb_pid == pid)
                {
                    break;
                }
                // free(patota_segmento_pcb);
            }

    return segmentos_patota;
}

t_segmento* retornar_segmento_tcb(t_list* segmentos_patota,int tid)
{
    return list_get(segmentos_patota,tid);
}

t_segmento* retornar_segmento_tareas(t_list* segmentos_patota)
{
    return list_get(segmentos_patota,list_size(segmentos_patota)-1);
}

t_segmento* retornar_segmento_pcb(t_list* segmentos_patota)
{
    return list_get(segmentos_patota,0);
}


/* -----------------------------Fin Tcb------------------------------- */

/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */

/* -----------------------------Impresiones--------------------------------------- */

/* ------------------------loggear enteros---------------------------- */
void imprimir_patotas_presentes()
{
    log_info(logger,"\t     En RAM se encuentran %d patotas",list_size(lista_de_patotas));
    pthread_mutex_lock(&sem_memoria);
    mostrar_fecha();
    // list_iterate(lista_de_patotas, (void*) iterator_segmento);
    list_iterate(lista_de_patotas, (void*) iterator_patotas_presentes);
    pthread_mutex_unlock(&sem_memoria);
}


void loggear_entero_en_char(char* entero)
{
    char* bytes = string_itoa(strlen(entero));
    log_info(logger,bytes);
    free(bytes);
}

void loggear_entero(int entero)
{
    char* bytes = string_itoa(entero);
    log_info(logger,bytes);
    free(bytes);
}

void loggear_entero_con_texto(char* texto,int entero)
{
    char buffer[100];
    char* bytes = string_itoa(entero);
    strcat(strcpy(buffer,texto),bytes);
    log_info(logger,buffer);
    free(bytes);
}

void loggear_linea()
{
    log_info(logger,"-------------------------------------------------------------");
}

void loggear_tcb (t_segmento* segmento_tcb)
{
    loggear_linea();
    uint32_t pos_x = retornar_pos_x_del_tcb(segmento_tcb);
    uint32_t pos_y = retornar_pos_y_del_tcb(segmento_tcb);
    log_info(logger,"Las posiciones en memoria son X|Y -> %d|%d",pos_x, pos_y);
    char estado = retornar_estado_del_tcb(segmento_tcb);
    log_info(logger,"El estado es -> %c",estado);
    loggear_linea();
}

/* ------------------------loggear segmentos---------------------------- */

void loggear_info_segmentos(t_segmento* segmento, char* nombre_de_segmento)
{
    pthread_mutex_lock(&mutex_segmentos_libres);
    loggear_entero_con_texto("Cantidad de bytes libres:",admin_segmentacion->bytes_libres);
    list_iterate(admin_segmentacion->segmentos_libres, (void*) iterator_segmento);
    log_info(logger,"El inicio del \"%s\" es %d, el fin %d y se encuentra en ram: %d \n",nombre_de_segmento, segmento->inicio, segmento->fin, segmento->se_encuentra);
    pthread_mutex_unlock(&mutex_segmentos_libres);
}

void loggear_info_segmento(t_segmento* segmento, char* nombre_de_segmento)
{
    log_info(logger,"El inicio del \"%s\" es %d, el fin %d y se encuentra en ram: %d \n",nombre_de_segmento, segmento->inicio, segmento->fin, segmento->se_encuentra);
}


/* ---------------------------Fin Impresiones------------------------------------- */

/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */

/* -----------Creaciones de Estructuras administrativas--------------- */

/* -------------Creacion de Patota--------------- */

void crear_patota(int pid,t_list* lista)
{
    t_pcb* t_pcb = malloc(sizeof(t_pcb));
    t_pcb->pid = recibir_catidad_de_tripulantes(lista);


    // t_list* lista = recibir_paquete(cliente_fd);

    // t_list* tareas = crear_tareas(lista);  EN ESTE MOMENTO NO ESTA SIENDO USADA DESCOMENTAR CUANDO SE NECESITE
    // list_iterate(tareas, (void*) iterator);

    
    // list_clean_and_destroy_elements(tareas, (void*) iterator_destroy);   
};

/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */

/* -----------Creacion de tareas--------------- */

t_list* crear_tareas(t_list* lista)
{
    pthread_mutex_lock(&sem_memoria); 
    // t_list* tareas = (t_tarea*) list_create();  initialization from incompatible pointer
    t_list* tareas = list_create();
    // t_list_iterator* list_iterator = (char*) list_iterator_create(lista);  idem tareas
    t_list_iterator* list_iterator = list_iterator_create(lista);
    // char** substrings;

    while(list_iterator_has_next(list_iterator))
    {
        t_tarea* tarea = malloc(sizeof(t_tarea));
        // substrings = string_split(list_iterator_next(list_iterator),"-");
        char* string = (char*) list_iterator_next(list_iterator);
        char** substrings = string_split(string,";");

        // Muestro lo que contienen los arrays, un dia con mas ganas crear una funcion que lo haga
        // log_info(logger,"Tarea Parametro: %s",substrings[0]);
        // log_info(logger,"Pos X: %s",substrings[1]);
        // log_info(logger,"Pos Y: %s",substrings[2]);
        // log_info(logger,"Tiempo: %s",substrings[3]);

        char** linea_tarea = string_split(substrings[0]," ");
        // log_info(logger,"Tarea: %s",linea_tarea[0]);
        // log_info(logger,"Parametro: %s",linea_tarea[1]);
        
        tarea->accion = malloc(strlen(linea_tarea[0])+1);
        memcpy(tarea->accion,linea_tarea[0], strlen(linea_tarea[0])+1);
        tarea->parametro = (uint32_t) atoi(linea_tarea[1]);
        tarea->posicion_x = (uint32_t) atoi(substrings[1]);
        tarea->posicion_y = (uint32_t) atoi(substrings[2]);
        tarea->tiempo = (uint32_t) atoi(substrings[3]);

        list_add(tareas,tarea);
        string_iterate_lines(substrings, iterator_lines_free);
        // string_iterate_lines(substrings, free);
        free(substrings);
        string_iterate_lines(linea_tarea, iterator_lines_free);
        // string_iterate_lines(linea_tarea, free);
        free(linea_tarea);
        pthread_mutex_unlock(&sem_memoria); 

    }

    list_iterator_destroy(list_iterator);

    return tareas;
};

char* unir_tareas(t_list* lista)
{
    // t_list* tareas = list_create();
    t_list_iterator* list_iterator = list_iterator_create(lista);
    

    // char* tareas_unidas="";
    char tareas_unidas[100];
    strcpy(tareas_unidas,"");
    while(list_iterator_has_next(list_iterator))
    {
        char* nueva_tarea = (char*) list_iterator_next(list_iterator);
        
        strcat(tareas_unidas,nueva_tarea);
        // calloc();

        // log_info(logger,"String: %s",tareas_unidas);
    }

    // log_info(logger,"Las tareas son: %d",strlen(tareas_unidas));
    list_iterator_destroy(list_iterator);

    strcat(tareas_unidas,"\0");

    char* tareas = (char*) malloc(strlen(tareas_unidas)+1);
    strcpy(tareas,tareas_unidas);

    tareas[strlen(tareas)] = '\0';

    return tareas;
};

int cantidad_de_tareas (char* tareas)
{
    int nro_de_tareas = 0;

    for (int i = 0; i < strlen(tareas); i++)
    {
        if(tareas[i] == '-') nro_de_tareas++;
    }
    
    return nro_de_tareas+1;
}

int cantidad_de_apariciones (char* string, char caracter)
{
    int nro_de_apariciones = 0;

    for (int i = 0; i < strlen(string); i++)
    {
        if(string[i] == caracter) nro_de_apariciones++;
    }
    
    return nro_de_apariciones;
}

/* -----------Creacion de tcbs--------------- */

t_list* crear_tcbs(t_list* lista,int cant_tripulantes)
{
    t_list* tripulantes_ubicados = recibir_posiciones_de_los_tripulantes(lista);

    t_list* lista_tcbs = list_create();
    for (int i = 1; i <= cant_tripulantes; i++)
    {
        t_tcb* tcb = malloc(sizeof(t_tcb));
        tcb->tid = (uint32_t) i;
        tcb->estado = (char) 'N';
        tcb->posicion_x = (uint32_t) 0;
        tcb->posicion_y = (uint32_t) 0;
        tcb->proxima_instruccion = (uint32_t) 0;
        tcb->puntero_pcb = (uint32_t) 0; // despues hay que actualizarlo con el byte del pcb donde este guardado

        list_add(lista_tcbs,(t_tcb*) tcb);
    }
    
    char* posiciones_del_tripulate;

    // list_iterate(tripulantes_ubicados,iterator_posicion);
    t_list_iterator* list_iterator = list_iterator_create(tripulantes_ubicados);


    while(list_iterator_has_next(list_iterator))
    {
        posiciones_del_tripulate = (char*) list_iterator_next(list_iterator);
        char** posiciones = string_split(posiciones_del_tripulate,"|");

        t_tcb* tcb_a_ubicar;
        tcb_a_ubicar = list_get(lista_tcbs,list_iterator->index);
        tcb_a_ubicar->posicion_x =  (uint32_t) atoi(posiciones[0]);
        tcb_a_ubicar->posicion_y = (uint32_t) atoi(posiciones[1]);

        string_iterate_lines(posiciones, iterator_lines_free);
        free(posiciones);
    }
    free(list_iterator);

    list_clean_and_destroy_elements(tripulantes_ubicados, (void*) iterator_lines_free);   
    free(tripulantes_ubicados);

    return lista_tcbs;
}

/* ---------Fin Creaciones de Estructuras administrativas------------- */

/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */


/* --------------------------Recibir---------------------------------- */

uint32_t recibir_pid(t_list* lista)
{
    // t_pcb* t_pcb = malloc(sizeof(t_pcb));
    char* pid_texto = list_remove(lista,0); 
    uint32_t pid_numero = (uint32_t) atoi(pid_texto);
    // log_info(logger,(uint32_t) t_pcb->pid_texto);
    // log_info(logger,(uint32_t) t_pcb->pid_texto);
    free(pid_texto);
    return pid_numero;
};

char recibir_estado(t_list* lista)
{
    char* estado = list_remove(lista,0);
    char est = estado[0];
    log_info(logger,"estado %c", est);
    free(estado);
    return est;
};

uint32_t recibir_catidad_de_tripulantes(t_list* lista)
{
    // t_pcb* t_pcb = malloc(sizeof(t_pcb));
    char* pid = list_remove(lista,0); 
    uint32_t cant_de_tripulantes = (uint32_t) atoi(pid);
    // log_info(logger,(uint32_t) t_pcb->pid);
    free(pid);
    return cant_de_tripulantes;
};


t_list* recibir_posiciones_de_los_tripulantes(t_list* lista)
{
    char* linea = list_remove(lista,list_size(lista)-1);
    // log_info(logger,linea);

    char** posiciones_por_tripulate = string_split(linea,";");
    // char** posiciones = string_split(posiciones_por_tripulate,"|");
    

    t_list* tripulantes_ubicados = list_create();
    for (int i = 0; posiciones_por_tripulate[i] != NULL; i++)
    {
        // log_info(logger,posiciones_por_tripulate[i]);
        list_add(tripulantes_ubicados,(uint32_t) posiciones_por_tripulate[i]);
    }

    free(posiciones_por_tripulate);
    free(linea);
    // list_iterator_destroy(list_iterator);
    return tripulantes_ubicados;
};


/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */

void* recibir_patota(int socket_cliente, t_log* logger,int* direccion_size)
{
	char* buffer = recibir_buffer(direccion_size, socket_cliente);
    // dentro del buffer tendremos un uint32_t que indicara la cant de tripulantes
    // luego contendra un paquete con todas la tareas
	log_info(logger, "Me llego el mensaje %s", buffer);
	free(buffer);
    return NULL;
};


/* ------------------------Fin Recibir-------------------------------- */


/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */

/* ------------------Finalizacion de proceso-------------------------- */

void terminar_miramhq(t_log* logger, t_config* config)
{
	log_destroy(logger);
	// liberar_conexion(conexion);
	config_destroy(config);
}

/* ----------------Fin de finalizacion de proceso--------------------- */

/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */

/* -------------------------Testeos----------------------------------- */

void validar_malloc()
{   
    // Como no se puede validar (no existe una funcion) metemos un valor
    // se debe ejecutar con ./vexec (valgrid) para saber si se produce un robo de memoria por derecha
    int bytes = config_get_int_value(config,"TAMANIO_MEMORIA"); //modificar a 7 el VALOR de TAMAIO_MEMORIA
    reservar_espacio_de_memoria(config);
    strcpy(MEMORIA, "abeef");  // se carga la ram  con 6 + '\0' bytes de datos
    // Decomentar para que verlo romper
    // strcpy(RAM, "abeefe");  // se carga la ram  con 7 + '\0' bytes de datos - este lo hace romper
    printf("bytes de la ram %d\n",bytes);
    printf("bytes de lo que esta cargado dentro de la ram %d\n",strlen(MEMORIA)+1);
    free(MEMORIA);
};

/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */

void testear_biblioteca_compartida()
{
    // se testea que haya vinculado el logger y funciona el shared
    log_info(logger, "Soy el Mi-RAMar en HD! %s", mi_funcion_compartida());
}

/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */

void testear_enviar_mensaje(int conexion)
{
    // se testea que haya vinculado el config
    char* mensaje = config_get_string_value(config,"CLAVE");
    log_info(logger,mensaje);
    // generamos la conexion
    // int conexion = crear_conexion((char*)config_get_string_value(config,"IP"),(char*)config_get_string_value(config,"PUERTO"));

    // enviamos un mensaje al servidor con la conexion que creamos
    enviar_mensaje(mensaje,conexion);
    // liberar_conexion(conexion);
}
/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */

void testear_asignar_y_liberar_segmentacion()
{
    t_segmento* segmento20,*segmento32;
    // t_segmento* segmento48;
    // segmento48 = buscar_segmento_libre(48);
    // printf("\tCantidad de bytes libres: %d\n",admin_segmentacion->bytes_libres);
    // list_iterate(admin_segmentacion->segmentos_libres, (void*) iterator_segmento);
    // log_info(logger,"El inicio del segmento 48 es %d y el fin %d\n", segmento48->inicio,segmento48->fin);
    // // // printf("El inicio del segmento 48 es %d y el fin %d\n",segmento48->inicio,segmento48->fin);
    // printf("\n");

 
    segmento20 = buscar_segmento_libre(20);
    printf("\tCantidad de bytes libres: %d\n",admin_segmentacion->bytes_libres);
    list_iterate(admin_segmentacion->segmentos_libres, (void*) iterator_segmento);
    log_info(logger,"El inicio del segmento 20 es %d y el fin %d\n", segmento20->inicio, segmento20->fin);
    // printf("El inicio del segmento 20 es %d y el fin %d\n",segmento20->inicio,segmento20->fin);
    printf("\n");

    segmento32 = buscar_segmento_libre(32);
    printf("\tCantidad de bytes libres: %d\n",admin_segmentacion->bytes_libres);
    list_iterate(admin_segmentacion->segmentos_libres, (void*) iterator_segmento);
    log_info(logger,"El inicio del segmento 32 es %d y el fin %d\n", segmento32->inicio,segmento32->fin);
    // printf("El inicio del segmento 32 es %d y el fin %d\n",segmento32->inicio,segmento32->fin);
    printf("\n");

    liberar_segmento(segmento20);
    printf("\tCantidad de bytes libres: %d\n",admin_segmentacion->bytes_libres);
    list_iterate(admin_segmentacion->segmentos_libres, (void*) iterator_segmento);
    // printf("El inicio nuevo es %d y el fin %d\n",segmento20->inicio,segmento20->fin);
    printf("\n");

    liberar_segmento(segmento32);
    printf("\tCantidad de bytes libres: %d\n",admin_segmentacion->bytes_libres);
    list_iterate(admin_segmentacion->segmentos_libres, (void*) iterator_segmento);
    printf("\n");

    // free(segmento48);
    // free(segmento32);
    // free(segmento20);
}


/* -------------------------Fin Testeos------------------------------- */

/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */

/* ------------------------------------------------------------------- */
/* -------------------------Iteradores-------------------------------- */
/* ------------------------------------------------------------------- */


void iterator_agregar_tareas(char* value)
{
    // char* string = string_new();
    char** substrings = string_split(value,"-");

    // should_string(substrings[0])

    string_iterate_lines(substrings, (void*) free);
    free(substrings);
};

void iterator_destroy(char* value)
{
    free(value);
};

void iterator_destroy_tcb(t_tcb* tcb)
{
    free(tcb);
};
void iterator_destroy_tarea(t_tarea* tarea)
{
    free(tarea->accion);
    free(tarea);
};

void iterator_lines_free(char* string)
{
    free(string);
};

void iterator_segmentos_free(t_segmento* segmento)
{
    free(segmento);
    // liberar_segmento(segmento);
};

void iterator_posicion(char* pos)
{
    log_info(logger,"Posicion X|Y: %s", pos);
};

void iterator_patotas_presentes_f(t_list* patota)
{
    t_segmento* segmento_pcb = list_get(patota,0);
    log_info(logger,"Patota con PID -> %d",retornar_pid_del_pcb(segmento_pcb));
}

void iterator_patotas_presentes(t_list* patota)
{
    t_segmento* segmento_pcb = list_get(patota,0);
    t_segmento* segmento_presente;
    t_list* segmentos_de_la_patota_presentes = list_filter(patota,condicion_segmento_presente_en_memoria);
    t_list_iterator* list_iterator_patota = list_iterator_create(segmentos_de_la_patota_presentes);
    while(list_iterator_has_next(list_iterator_patota))
    {
        segmento_presente = (t_segmento*) list_iterator_next(list_iterator_patota);
        int tamanio = segmento_presente->fin - segmento_presente->inicio +1;
        log_info(logger,"Proceso: %d\t Segmento: %d\t Inicio: %d\t Tam: %d bytes",retornar_pid_del_pcb(segmento_pcb),list_iterator_patota->index+1,segmento_presente->inicio,tamanio);
        
    }
    free(list_iterator_patota);
    // list_clean(segmentos_de_la_patota_presentes);
    list_destroy(segmentos_de_la_patota_presentes);
}

void mostrar_fecha()
{
    char* times_test = malloc(sizeof(char) * 16);
	time_t ltime;
	ltime=time(NULL);
	struct tm *tm;
	tm=localtime(&ltime);

	sprintf(times_test,"Dump %02d/%02d/%04d %02d:%02d:%02d", tm->tm_mday, tm->tm_mon, 
    tm->tm_year+1900, tm->tm_hour, tm->tm_min, tm->tm_sec);
	log_info(logger,"\t\t%s",times_test);
    // free(times_test);
}

void iterator_pcb(t_pcb* pcb)
{
    log_info(logger,"\tPID: %d", pcb->pid);
    log_info(logger,"Tareas: %d", pcb->tareas);
};

void iterator_tcb(t_tcb* tcb)
{
    log_info(logger,"\tTID: %d", tcb->tid);
    log_info(logger,"Estado: %c", tcb->estado);
    log_info(logger,"Posicion X: %d", tcb->posicion_x);
    log_info(logger,"Posicion Y: %d", tcb->posicion_y);
    log_info(logger,"Proxima Instruccion: %d", tcb->proxima_instruccion);
    log_info(logger,"PCB: %d", tcb->puntero_pcb);
};

void iterator_tarea(t_tarea* tarea)
{
    log_info(logger,"Tarea: %s", tarea->accion);
    // uint32_t numero = (uint32_t) string_itoa(tarea->parametro);
    // log_info(logger, numero);
    log_info(logger,"Parametro: %d", tarea->parametro);
    log_info(logger,"Pos X %d y Pos Y: %d", tarea->posicion_x,tarea->posicion_y);
    log_info(logger,"Tiempo: %d", tarea->tiempo);
    // free(numero);
};

void iterator_segmento(t_segmento* segmento_libre)
{
    log_info(logger,"Inicio: %d", segmento_libre->inicio);
    log_info(logger,"Fin: %d", segmento_libre->fin);
    log_info(logger,"Tipo: %c", segmento_libre->tipo);
    log_info(logger,"Se encuentra en RAM: %d",segmento_libre->se_encuentra);
}

void iterator_patota(t_list* segmentos_patota)
{   
    t_segmento* segmento_pcb, *segmento_tcb, *segmento_tareas;
    loggear_linea();
    segmento_pcb = retornar_segmento_pcb(segmentos_patota);
    log_info(logger,"PCB:%d", retornar_pid_del_pcb(segmento_pcb));
    // list_iterate(segmento_pcb, (void*) iterator_segmento);
    loggear_info_segmento(segmento_pcb,"segmento_pcb");

    for (int i = 1; i < list_size(segmentos_patota)-1; i++)
    {
        
        segmento_tcb = retornar_segmento_tcb(segmentos_patota,i);
        if (segmento_tcb->se_encuentra != 1)
        {
            log_info(logger,"TCB: %d - Se encuentra expulsado\n", i);
        }else
        {
            // log_info(logger,"TCB-%d: %d", i, retornar_tid_del_tcb(segmento_tcb));
            log_info(logger,"TCB: %d", i, retornar_tid_del_tcb(segmento_tcb));
            log_info(logger,"Estado: %c", retornar_estado_del_tcb(segmento_tcb));
            loggear_info_segmento(segmento_tcb,"segmento_tcb");

        }
        
        // log_info(logger,"PCB: %d", retornar_puntero_pid_del_tcb(segmento_tcb));
        // t_tcb* tcb;
        // tcb = retornar_tcb(segmento_tcb);
        // log_info(logger,"PCB: %d", tcb->posicion_x);

        // iterator_tcb(tcb);
    }
    segmento_tareas = retornar_segmento_tareas(segmentos_patota);
    char* works = retornar_tareas(segmento_tareas);
    log_info(logger,"Las taeritas son: %s",works);
    free(works);
    loggear_info_segmento(segmento_tareas,"segmento_tareas");
    loggear_linea();
}

bool orden_lista_admin_segmentacion(t_segmento* segmento_libre_A, t_segmento* segmento_libreB)
{
    return segmento_libre_A->inicio < segmento_libreB->inicio;
}

bool orden_lista_admin_segmentacion_best_free(t_segmento* segmento_libre_A, t_segmento* segmento_libre_B)
{
    int diferencia_A = segmento_libre_A->fin - segmento_libre_A->inicio + 1;
    int diferencia_B = segmento_libre_B->fin - segmento_libre_B->inicio + 1;
    return diferencia_A < diferencia_B;
}

bool orden_lista_pcbs(t_pcb* pcb_a, t_pcb* pcb_b)
{
    return pcb_a->pid < pcb_b->pid;
}

bool orden_lista_segmentos(t_segmento* seg_a, t_segmento* seg_b)
{
    return seg_a->inicio < seg_b->inicio;
}

bool comparador_patotas(t_list* seg_patota_a, t_list* seg_patota_b)
{
    t_pcb* pcb_a, *pcb_b;
    pcb_a = list_get(seg_patota_a,0);
    pcb_b = list_get(seg_patota_b,0);
    return pcb_a->pid > pcb_b->pid;
}

bool condicion_segmento_afectado(t_segmento* seg)
{
    return seg->fin > ultimo_segmento_libre_compactable()->inicio;
}

bool condicion_segmento_presente_en_memoria(t_segmento* seg)
{
    return seg->se_encuentra == 1;
}

void transformacion_segmento_afectado(t_segmento* seg)
{
    int diferencia_libre = ultimo_segmento_libre_compactable()->fin - ultimo_segmento_libre_compactable()->inicio + 1;
    int diferencia_ocupado = seg->fin - seg->inicio + 1;

    // switch (seg->tipo)
    // {
    // case 'P':;
    //     uint32_t pid_afectado = retornar_pid_del_pcb(seg);
    //     uint32_t tareas_afectadas = retornar_inicio_de_tareas_del_pcb(seg);
    //     t_pcb* pcb_afectado;
    //     pcb_afectado->pid = pid_afectado;
    //     pcb_afectado->tareas = tareas_afectadas;
    //     guardar_en_MEMORIA_pcb(seg,pcb_afectado);
    //     break;
    // case 'T':
    //     uint32_t tid_afectado = retornar_tid_del_tcb(seg);
    //     uint32_t tid_afectado = retornar_tid_del_tcb(seg);
    //     uint32_t tareas_afectadas = retornar_inicio_de_tareas_del_pcb(seg);
    //     break;
    // default:
    //     break;
    // }
    memcpy(MEMORIA + (seg->inicio) - diferencia_libre, (MEMORIA + seg->inicio),diferencia_ocupado);
    seg->inicio -= diferencia_libre;
    seg->fin -= diferencia_libre;

    ultimo_segmento_libre_compactable()->inicio += diferencia_ocupado;
    ultimo_segmento_libre_compactable()->fin += diferencia_ocupado;
    consolidar_segmentos_libres_se_es_posbile();
}

bool final_maximo(t_segmento* seg_a, t_segmento* seg_b)
{
    return (seg_a->fin >= seg_b->fin);
}


bool mismo_pid(t_list* lista_patota_A,t_list* lista_patota_B)
{
    t_pcb* pcb_A = list_get(lista_patota_A,0);
    t_pcb* pcb_B = list_get(lista_patota_B,0);
    return pcb_A->pid == pcb_B->pid;
}

void iterator_tarea_cargar_a_memoria(t_tarea* tarea)
{
    // Buscar proximo hueco libre -- HACER FUNCION
    int desplazamiento = 0;
    char delimitador_entre_accion_parametro = (char) ' ';
    char delimitador_entre_tareas = (char) ';';
    memcpy(MEMORIA + desplazamiento, (void*) (&tarea->accion), strlen(tarea->accion));
    desplazamiento += strlen(tarea->accion);
    memcpy(MEMORIA + desplazamiento, (void*) (&delimitador_entre_accion_parametro), sizeof(char));
    desplazamiento += sizeof(char);
    memcpy(MEMORIA + desplazamiento, (void*) (&tarea->parametro), sizeof(char));
    desplazamiento += sizeof(char);
    memcpy(MEMORIA + desplazamiento, (void*) (&delimitador_entre_tareas), sizeof(char));
    desplazamiento += sizeof(char);
    memcpy(MEMORIA + desplazamiento, (void*) (&tarea->posicion_x), sizeof(char));
    desplazamiento += sizeof(char);
    memcpy(MEMORIA + desplazamiento, (void*) (&delimitador_entre_tareas), sizeof(char));
    desplazamiento += sizeof(char);
    memcpy(MEMORIA + desplazamiento, (void*) (&tarea->posicion_y), sizeof(char));
    desplazamiento += sizeof(char);
    memcpy(MEMORIA + desplazamiento, (void*) (&delimitador_entre_tareas), sizeof(char));
    desplazamiento += sizeof(char);
    memcpy(MEMORIA + desplazamiento, (void*) (&tarea->tiempo), sizeof(char));
    desplazamiento += sizeof(char);

};

void iterator(char* value)
{
    log_info(logger,value);
};


/* ------------------------Fin Iteradores----------------------------- */

/* ------------------------------------------------------------------- */
/* --------------------------Signals---------------------------------- */
/* ------------------------------------------------------------------- */

/* ---------------------------Envios---------------------------------- */

void my_signal_kill(int sig)
{
    loggear_linea();
    // cod_cierre = 1;
    log_info(logger,"Se utilizo el signal %d correctamente", sig);
    // list_iterate(lista_de_segmentos_ocupados, (void*) iterator_segmento);
    t_list* lista = lista_de_segmentos_ocupados();
    list_clean_and_destroy_elements(lista, (void*) iterator_destroy);
    // free(lista);
    list_destroy(lista);
    loggear_linea();
}

void my_signal_compactar(int sig)
{
    loggear_linea();
    log_info(logger,"Se utilizo el signal %d correctamente", sig);
    log_info(logger,"Se procedera a compactar, espere por favor");
    pthread_mutex_lock(&sem_memoria);
    compactar();
    pthread_mutex_unlock(&sem_memoria);
    loggear_linea();
}



/* ------------------------------------------------------------------- */
/* ---------------------------Envios---------------------------------- */
/* ------------------------------------------------------------------- */

void enviar_proxima_tarea(char* tarea_solicitada, int cliente_fd)
{
		// int cod_operacion = FINALIZACION; 
		int cod_operacion = ENVIAR_PROXIMA_TAREA; 
	t_paquete* paquete = crear_paquete(cod_operacion);
	
	log_info(logger,tarea_solicitada);
	agregar_a_paquete(paquete,tarea_solicitada,strlen(tarea_solicitada)+1);

	enviar_paquete(paquete,cliente_fd);
	eliminar_paquete(paquete);
}

/* ------------------------------------------------------------------- */
/* --------------------------Archivos--------------------------------- */
/* ------------------------------------------------------------------- */

void leer_archivo(char* path)
{
    FILE* archivo = fopen(path,"r");
    char line[100];
    
    if (archivo == NULL)
    {
        perror("Unable to open th file");
        exit(1);
    }
    
    while(fgets(line,sizeof(line),archivo))
    {
        printf("%s",line);
        // log_info(logger,line);
    }
    fclose(archivo);
}