#include "miramhq.h"

#define SMOBJ_MEMORIA "/miramhq"
#define ASSERT_CREATE(AmongOS, id, err)                                                   \
    if(err) {                                                                           \
        nivel_destruir(AmongOS);                                                          \
        nivel_gui_terminar();                                                           \
        fprintf(stderr, "Error al crear '%c': %s\n", id, nivel_gui_string_error(err));  \
        return EXIT_FAILURE;                                                            \
    }

/*
 * @NAME: rnd
 * @DESC: Retorna un entero en el rango [-1, 1]
 */
int rnd() {
	return (rand() % 3) - 1;
}

t_log* logger;
t_config* config;
administrador_de_segmentacion* admin_segmentacion;
t_list* lista_de_patotas;
t_list* list_dump_segmentacion;
pthread_mutex_t sem_memoria, mutex_segmentos_libres, mutex_mapa, mutex_paginacion;
int SMOBJ_SIZE;
int cod_cierre;
NIVEL* AmongOS;
char* CRITERIO;

int x_max, y_max;
int cols, rows;
int err;


/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */


int main(int argc, char ** argv){


    /* -------------- Variables / Estructuras Adminisdtrativas ---------------- */
    // creacion del logger y del config
    logger = iniciar_logger("miramhq");
    config = leer_config("miramhq");

    // imprime si alguno de los dos no se pudo crear
    validar_logger(logger);
    validar_config(config);

    /* -------------------------Primer Paso----------------------------------- */
    // Reservar espacio de memoria
    reservar_espacio_de_memoria();



    char* esquema = config_get_string_value(config,"ESQUEMA_MEMORIA");
    if ( strcmp(esquema,"SEGMENTACION") == 0)
    {
        
        iniciar_segmentacion();
    }else
    {
        iniciar_paginacion();
    }


    // testear_asignar_y_liberar_segmentacion();
    // liberar_espacio_de_memoria();
    /* -----------------------Fin Primer Paso--------------------------------- */
    /* -------------------------Segundo Paso----------------------------------- */
    // Crear el mapa
    // pthread_t hilo_mapa;
	// pthread_create(&hilo_mapa,NULL, (void*) iniciar_mapa,NULL);
	// pthread_detach(hilo_mapa);
    
    iniciar_mapa();

    /* -----------------------Fin Segundo Paso-------------------------------- */
    /* -------------------------Tercer Paso----------------------------------- */

    // Iniciar Servidor
    int server_fd = iniciar_servidor(logger,config);

    signal(SIGCHLD,&my_signal_kill);
    signal(SIGHUP,&my_signal_compactar);
    cod_cierre = 0;

    pthread_mutex_init(&sem_memoria,NULL);
    pthread_mutex_init(&mutex_mapa,NULL);

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
    /* -----------------------Fin Tercer Paso-------------------------------- */

    // terminamos el proceso, eliminamos todo
    eliminar_mapa();
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
        atender_cliente_PAGINACION(cliente_fd);
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
			// printf("Me llegaron los siguientes valores:\n");
			list_iterate(lista, (void*) iterator);
			break;
		case INICIAR_PATOTA:
            // loggear_linea();
            // log_info(logger,"Entro a INICIAR PATOTA");

			recibir_paquete(cliente_fd,lista);

    /* ----------------------Estrucutras administrativas-------------------------- */
    /* -----------------------------Recibir pid----------------------------------- */
            uint32_t pid = recibir_pid(lista);
                // Descomentar para testear
            // log_info(logger,"\t El pid de la patota es: %d\n",pid);
            t_pcb* pcb = malloc(sizeof(t_pcb));
            pcb->pid = pid;
    
    /* ------------------------Cantidad de tripulantes---------------------------- */
            int cant_tripulantes = recibir_catidad_de_tripulantes(lista);

    /* ------------------------Creacion de tcbs---------------------------- */
            t_list* tcbs = crear_tcbs(lista,cant_tripulantes);
            pthread_mutex_lock(&mutex_mapa);
            iniciar_patota_en_mapa(pid,tcbs);
            pthread_mutex_unlock(&mutex_mapa);
    /* ------------------------Creacion de tareas---------------------------- */

            char* tareas_unidas = unir_tareas(lista);
                // Descomentar para testear
            // loggear_linea();
            // loggear_entero(strlen(tareas_unidas));
            // log_info(logger,"Tareas unidas retornadas-> %s",tareas_unidas);
            
            iniciar_patota_SEGMENTACION (lista, pcb, tcbs, tareas_unidas, cant_tripulantes);

            // mostrar_patotas_presentes_en_mapa();
    /*-----------------------Liberacion de estructuras---------------------------*/
            free(pcb);
            list_clean_and_destroy_elements(lista, (void*) iterator_destroy);
            list_clean_and_destroy_elements(tcbs, (void*) iterator_destroy_tcb);
			list_destroy(tcbs);
            free(tareas_unidas);
            break;
		case ENVIAR_PROXIMA_TAREA:
        // DISCORDIADOR me manda un pid y un tid
            // loggear_linea();
            // log_info(logger,"Entro a proxima tarea");

            recibir_paquete(cliente_fd,lista);

            // loggear_linea();
            uint32_t pid_prox_tarea = recibir_pid(lista);
            uint32_t tid_prox_tarea = recibir_pid(lista);
            
            // loggear_entero(pid_prox_tarea);
            // loggear_entero(tid_prox_tarea);

            char* tarea_solicitada = enviar_proxima_tarea_SEGMENTACION(pid_prox_tarea,tid_prox_tarea);
            // loggear_entero(strlen(tarea_solicitada));
            // log_info(logger,"Tarea solicitada-> %s",tarea_solicitada);
            enviar_mensaje(tarea_solicitada,cliente_fd);

            free(tarea_solicitada);        
            list_clean_and_destroy_elements(lista, (void*) iterator_destroy);
			break;
        case ACTUALIZAR_ESTADO:
        // DISCORDIADOR me manda un pid y un tid
            recibir_paquete(cliente_fd,lista);
            // loggear_linea();
            // log_info(logger,"Entro a Actualizar estado");

            // loggear_linea();
            uint32_t pid_actualizar_estado = recibir_pid(lista);
            uint32_t tid_actualizar_estado = recibir_pid(lista);
            char estado_actualizar_estado = recibir_estado(lista);
	        // log_info(logger,"estado %c",estado_actualizar_estado);
        
            actualizar_estado_SEGMENTACION(pid_actualizar_estado,tid_actualizar_estado,estado_actualizar_estado);
            
            // liberar estructuras utilizadas
            list_clean_and_destroy_elements(lista, (void*) iterator_destroy);
            break;
        case EXPULSAR_TRIPULANTE:;
            // loggear_linea();
            // log_info(logger,"Entro a Expulsar");

            // pthread_mutex_lock(&mutex_segmentos_libres);
            // pthread_mutex_lock(&sem_memoria); 
            
            recibir_paquete(cliente_fd,lista);

            uint32_t pid_expulsar_tripulante = recibir_pid(lista);
            uint32_t tid_expulsar_tripulante = recibir_pid(lista);
            // loggear_entero(pid_expulsar_tripulante);
            // loggear_entero(tid_expulsar_tripulante);
            // char aux_expulsar = obtener_id_mapa(pid_expulsar_tripulante,tid_expulsar_tripulante);
            pthread_mutex_lock(&mutex_mapa);
            expulsar_tripulante_en_mapa(pid_expulsar_tripulante,tid_expulsar_tripulante);
            pthread_mutex_unlock(&mutex_mapa);

            
            expulsar_tripulante_SEGMENTACION(pid_expulsar_tripulante,tid_expulsar_tripulante);
            // pthread_mutex_unlock(&sem_memoria); 
            // pthread_mutex_unlock(&mutex_segmentos_libres);
            break;
        case COMPACTAR:
            pthread_mutex_lock(&mutex_segmentos_libres);
            pthread_mutex_lock(&sem_memoria); 
            pthread_mutex_lock(&mutex_mapa); 

            compactar();

            pthread_mutex_unlock(&mutex_segmentos_libres);
            pthread_mutex_unlock(&sem_memoria); 
            pthread_mutex_unlock(&mutex_mapa); 
            // list_iterate(admin_segmentacion->segmentos_libres,iterator_segmento);

            break;
        case DUMP:
            
            pthread_mutex_lock(&sem_memoria);
            pthread_mutex_lock(&mutex_segmentos_libres);
            pthread_mutex_lock(&mutex_mapa);
            // getyx(stdscr,);
            // move(y_max-2,0);
            // mvprintw(y_max-2,1,"%s","Patotas presentes ->");
            // clrtoeol();
            // mostrar_patotas_presentes_en_mapa();
            hacer_dump_SEGMENTACION();

            pthread_mutex_unlock(&mutex_mapa);
            pthread_mutex_unlock(&mutex_segmentos_libres);
            pthread_mutex_unlock(&sem_memoria);

            break;
        case RECIBIR_LA_UBICACION_DEL_TRIPULANTE:
            // loggear_linea();
            // log_info(logger,"Entro a cambiar ubicacion");

            recibir_paquete(cliente_fd,lista);
            uint32_t pid_ubicacion = recibir_pid(lista);
            uint32_t tid_ubicacion = recibir_pid(lista);


            uint32_t nueva_pos_x = recibir_pid(lista);
            uint32_t nueva_pos_y = recibir_pid(lista);
            pthread_mutex_lock(&mutex_mapa);
            mover_personaje_en_mapa(pid_ubicacion,tid_ubicacion,nueva_pos_x,nueva_pos_y);
            pthread_mutex_unlock(&mutex_mapa);

            cambiar_ubicacion_tripulante_SEGMENTACION(pid_ubicacion,tid_ubicacion,nueva_pos_x,nueva_pos_y);

            break;
		case -1:
			log_error(logger, "el cliente se desconecto. Terminando servidor");
			return EXIT_FAILURE;
		default:
			log_warning(logger, "Operacion desconocida. No quieras meter la pata");
			break;
		}

        list_destroy(lista);
        pthread_exit(pthread_self);
}


void atender_cliente_PAGINACION(int cliente_fd)
{
        int cod_op = recibir_operacion(cliente_fd);
        
        t_list* lista = list_create();



        switch(cod_op)
		{
		case INICIAR_PATOTA:

			recibir_paquete(cliente_fd,lista);

    /* ----------------------Estrucutras administrativas-------------------------- */
    /* -----------------------------Recibir pid----------------------------------- */
            uint32_t pid = recibir_pid(lista);
            t_pcb aux;
            aux.pid = pid;
            aux.tareas = 0;

    /* ------------------------Cantidad de tripulantes---------------------------- */
            int cant_tripulantes = recibir_catidad_de_tripulantes(lista);
            // loggear_entero(cant_tripulantes);

    /* ------------------------Creacion de tcbs---------------------------- */
            t_list* tcbs = crear_tcbs(lista,cant_tripulantes);
            // loggear_entero(list_size(tcbs));
            pthread_mutex_lock(&mutex_mapa);
            iniciar_patota_en_mapa(pid,tcbs);
            pthread_mutex_unlock(&mutex_mapa);


    /* ------------------------Creacion de tareas---------------------------- */
            char* tareas_unidas = unir_tareas(lista);
            // loggear_entero(strlen(tareas_unidas)+1);

    /* ------------------------PAGINACION---------------------------- */

            estructura_administrativa_paginacion admin_paginacion;
            admin_paginacion.pcb = aux;
            admin_paginacion.lista_de_tcb = tcbs;
            admin_paginacion.lista_de_tareas = tareas_unidas;
            iniciar_patota_PAGINACION(&admin_paginacion);

            list_clean_and_destroy_elements(lista, (void*) iterator_destroy);
            list_clean_and_destroy_elements(tcbs, (void*) iterator_destroy_tcb);
			list_destroy(tcbs);
            free(tareas_unidas);
            // mostrar_tabla_de_paginas();
            break;
		case ENVIAR_PROXIMA_TAREA:
        // DISCORDIADOR me manda un pid y un tid
            recibir_paquete(cliente_fd,lista);

            uint32_t pid_prox_tarea = recibir_pid(lista);
            uint32_t tid__prox_tarea = recibir_pid(lista);

            char* tarea_solicitada = proxima_tarea_PAGINACION(pid_prox_tarea,tid__prox_tarea);

            enviar_mensaje(tarea_solicitada,cliente_fd);

            free(tarea_solicitada);
            list_clean_and_destroy_elements(lista, (void*) iterator_destroy);

			break;
        case ACTUALIZAR_ESTADO:
        // DISCORDIADOR me manda un pid y un tid
            recibir_paquete(cliente_fd,lista);

            // loggear_linea();
            uint32_t pid_actualizar_estado = recibir_pid(lista);
            uint32_t tid_actualizar_estado = recibir_pid(lista);
            char estado_actualizar_estado = recibir_estado(lista);
            
           
            actualizar_estado_PAGINACION(pid_actualizar_estado,tid_actualizar_estado,estado_actualizar_estado);
         

            list_clean_and_destroy_elements(lista, (void*) iterator_destroy);
            
            break;
        case EXPULSAR_TRIPULANTE:
            recibir_paquete(cliente_fd,lista);

            uint32_t pid_expulsar_tripulante = recibir_pid(lista);
            uint32_t tid_expulsar_tripulante = recibir_pid(lista);

            // char aux_expulsar = obtener_id_mapa(pid_expulsar_tripulante,tid_expulsar_tripulante);
            pthread_mutex_lock(&mutex_mapa);
            expulsar_tripulante_en_mapa(pid_expulsar_tripulante,tid_expulsar_tripulante);
            pthread_mutex_unlock(&mutex_mapa);

            expulsar_tripulante_PAGINACION(pid_expulsar_tripulante,tid_expulsar_tripulante);

            break;
        case RECIBIR_LA_UBICACION_DEL_TRIPULANTE:
            recibir_paquete(cliente_fd,lista);
            uint32_t pid_ubicacion = recibir_pid(lista);
            uint32_t tid_ubicacion = recibir_pid(lista);

            uint32_t nueva_pos_x = recibir_pid(lista);
            uint32_t nueva_pos_y = recibir_pid(lista);

            pthread_mutex_lock(&mutex_mapa);
            mover_personaje_en_mapa(pid_ubicacion,tid_ubicacion,nueva_pos_x,nueva_pos_y);
            pthread_mutex_unlock(&mutex_mapa);

            cambiar_ubicacion_tripulante_PAGINACION(pid_ubicacion,tid_ubicacion,nueva_pos_x,nueva_pos_y);
            
            break;
        case DUMP:
            
            pthread_mutex_lock(&sem_memoria);
            pthread_mutex_lock(&mutex_mapa);
            
            hacer_dump_PAGINACION();
            
            pthread_mutex_unlock(&mutex_mapa);
            pthread_mutex_unlock(&sem_memoria);

            break;
		case -1:
			log_error(logger, "el cliente se desconecto. Terminando servidor");
			return EXIT_FAILURE;
		default:
			log_warning(logger, "Operacion desconocida. No quieras meter la pata");
			break;
		}

        list_destroy(lista);
        pthread_exit(pthread_self);
}


/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */

/* -----------------------------MAPA---------------------------------- */

void iniciar_mapa()
{
    initscr();
    if( !has_colors() )
    {
        endwin();
        fprintf(stderr, "Error - El terminal que estas utilizando no soporta colores\n");
        exit(1);
    }
    if( start_color() != OK )
    {
        endwin();
        fprintf(stderr, " Error - No se pudo inicializar los colores\n");
        exit(1);
    }

    // init_pair(1,COLOR_WHITE,COLOR_BLUE);
	// init_pair(2,COLOR_BLUE,COLOR_MAGENTA);
	// init_pair(3,COLOR_WHITE,COLOR_CYAN);
	// init_pair(4,COLOR_CYAN,COLOR_CYAN);
	// init_pair(5,COLOR_MAGENTA,COLOR_BLACK);
	// init_pair(6,COLOR_RED,COLOR_BLACK);
	// init_pair(7,COLOR_YELLOW,COLOR_BLACK);
	init_pair(97,COLOR_MAGENTA,COLOR_BLACK);
	init_pair(98,COLOR_RED,COLOR_BLACK);
	init_pair(99,COLOR_MAGENTA,COLOR_BLACK);

    getmaxyx(stdscr,y_max,x_max);
    // nivel_gui_inicializar();
	nivel_gui_get_area_nivel(&cols, &rows);

    attron(COLOR_PAIR(98) | A_BOLD);
    mvprintw(0,(x_max/2-(strlen("AmonOS")/2)-2),"%s","AmongOS");
    attroff(COLOR_PAIR(98) | A_BOLD);
    refresh();
    // attron(COLOR_PAIR(99) | A_BOLD);
    // mvprintw(y_max-3,5,"%s","Referencias");
    // attroff(COLOR_PAIR(99) | A_BOLD);
    // refresh();
}

void eliminar_mapa()
{
    nivel_gui_terminar();
}

void crear_personaje(char id, uint32_t pos_x, uint32_t pos_y)
{
	init_pair(65,COLOR_BLACK,COLOR_WHITE);
	init_pair(66,COLOR_BLUE,COLOR_CYAN);
	init_pair(67,COLOR_CYAN,COLOR_WHITE);
	init_pair(68,COLOR_CYAN,COLOR_BLACK);
    
    // attroff(A_BOLD);
    // attrset(COLOR_PAIR(5));
    // err = personaje_crear(AmongOS, id, pos_x, pos_y);
    // int nro = id;
    // printf("%d",nro);
    // sleep(1);
    // attrset(COLOR_PAIR(id)  | A_BOLD);
    mvprintw(pos_x,pos_y,"%c",id);
    
    // ASSERT_CREATE(AmongOS, id, err);
    
    // mvprintw(5+1,25,"El tripulante es %d", id);
    // nivel_gui_dibujar(AmongOS);
    refrescar_tabla_de_mapas();
    // refresh();
}

void mostrar_patotas_presentes_en_mapa()
{
    move(y_max-2,0);
    mvprintw(y_max-2,(x_max/2 - 13),"%s","Patotas presentes ->");
    clrtoeol();
    // init_pair(1,COLOR_WHITE,COLOR_BLUE);
	// init_pair(2,COLOR_BLUE,COLOR_MAGENTA);
	// init_pair(3,COLOR_WHITE,COLOR_CYAN);
	// init_pair(4,COLOR_CYAN,COLOR_BLACK);
    
    uint32_t ultimo_pid = -1;
    list_sort(TABLA_DE_MAPA,orden_lista_mapa);
    t_list_iterator* list_iterator_mapa = list_iterator_create(TABLA_DE_MAPA);
    
    int espacio = 0;
    while(list_iterator_has_next(list_iterator_mapa))
    {
        
        t_mapa* tripulante_en_mapa = (t_mapa*) list_iterator_next(list_iterator_mapa);
        if (tripulante_en_mapa->pid != ultimo_pid)
        {
            attron(COLOR_PAIR(tripulante_en_mapa->pid) | A_BOLD);
            mvprintw(y_max-2,(x_max/2+7+1+(2*espacio)),"%d",tripulante_en_mapa->pid);
            attroff(COLOR_PAIR(tripulante_en_mapa->pid) | A_BOLD);

            ultimo_pid = tripulante_en_mapa->pid;
            espacio++;
        }
        
    }
    list_iterator_destroy(list_iterator_mapa);

}


void mover_personaje(char id, uint32_t pos_x, uint32_t pos_y)
{
    err = item_desplazar(AmongOS, id, pos_x, pos_y);
    refresh();
    nivel_gui_dibujar(AmongOS);
}

void expulsar_tripulante_en_mapa(uint32_t pid, uint32_t tid)
{
    bool matchear_tripulante(t_mapa* tripulante)
    {
        return (tripulante->pid == pid && tripulante->tid == tid);
    };
    t_mapa* tcb_mapa = list_remove_by_condition(TABLA_DE_MAPA, matchear_tripulante);
    free(tcb_mapa);
    refrescar_tabla_de_mapas();
}


void mover_personaje_en_mapa(uint32_t pid,uint32_t tid, uint32_t pos_x_nuevo, uint32_t pos_y_nuevo)
{
    // bool matchear_tripulante(t_mapa* tripulante)
    // {
    //     return (tripulante->pid == pid && tripulante->tid == tid);
    // };

    // t_mapa* tcb_mapa = list_remove_by_condition(TABLA_DE_MAPA, matchear_tripulante);
    // strcpy(tcb_mapa->pos_x,pos_x_nuevo);
    // strcpy(tcb_mapa->pos_y,pos_y_nuevo);
    
    t_mapa* tripulante_en_mapa;

    t_list_iterator* list_tripulantes_en_mapa = list_iterator_create(TABLA_DE_MAPA);
    while(list_iterator_has_next(list_tripulantes_en_mapa))
    {
        tripulante_en_mapa = (t_mapa*) list_iterator_next(list_tripulantes_en_mapa);
        if (tripulante_en_mapa->pid == pid && tripulante_en_mapa->tid == tid )
        {
            tripulante_en_mapa->pos_x = pos_x_nuevo;
            tripulante_en_mapa->pos_y = pos_y_nuevo;
            list_replace(TABLA_DE_MAPA,list_tripulantes_en_mapa->index,tripulante_en_mapa);
        }
        
    }
    free(list_tripulantes_en_mapa);

    
    // tcb_mapa->pos_x = pos_x_nuevo;
    // tcb_mapa->pos_y = pos_y_nuevo;
    // list_add(TABLA_DE_MAPA,tcb_mapa);
    // list_sort(TABLA_DE_MAPA,orden_lista_mapa);
 
    refrescar_tabla_de_mapas();
}

void eliminar_personaje(char id)
{
    mvprintw(1,5,"%s"," ");
    refresh();
    // item_borrar(AmongOS,id);
    // refresh();
    // nivel_gui_dibujar(AmongOS);
}

void eliminar_personaje_ubicado(char id, uint32_t pos_x, uint32_t pos_y)
{
    
    mvprintw(pos_x,pos_y,"%s"," ");
    refresh();
}

void refrescar_tabla_de_mapas()
{
    // clrtobot();
    // clrtobot();
    // erase();
    // clear();
    // getmaxyx(stdscr,y_max,x_max);
	// nivel_gui_get_area_nivel(&cols, &rows);
    // sleep(1);


    for (int i = 6; i <= x_max-7; i++) {
        for (int j = 1; j <= y_max - 4; j++) {
            if (i == 6 || i == x_max-7){
                mvprintw(j,i,"%c",'*');
            }else{
                mvprintw(j,i,"%c",' ');
            }
            if (j == 1 || j == y_max-4)
            {
                mvprintw(j,i,"%s","* ");
            }
            
        }

        mvprintw(0,i,"%s","\n");
    }

    attron(COLOR_PAIR(98) | A_BOLD);
    mvprintw(0,3,"%s","1C - 2021");
    mvprintw(0,(x_max-25),"%s","TP - Sistemas Operativos");
    mvprintw(0,(x_max/2-(strlen("AmonOS")/2)-2),"%s","AmongOS");
    attroff(COLOR_PAIR(98) | A_BOLD);
    attron(COLOR_PAIR(97) | A_BOLD);
    mvprintw(y_max-1,(x_max-(17)),"%s","Quinta-Recursada");
    attroff(COLOR_PAIR(97) | A_BOLD);
    attron(COLOR_PAIR(99) | A_BOLD);
    mvprintw(y_max-3,x_max/2-6,"%s","Referencias");
    attroff(COLOR_PAIR(99) | A_BOLD);


    int inicio_x=7, inicio_y=2;
    t_list_iterator* tripulantes_en_mapa = list_iterator_create(TABLA_DE_MAPA);
    
    while(list_iterator_has_next(tripulantes_en_mapa))
    {
        t_mapa* tripulante = (t_mapa*) list_iterator_next(tripulantes_en_mapa);

        attron(COLOR_PAIR(tripulante->pid) | A_BOLD);
        mvprintw(inicio_y+tripulante->pos_y,inicio_x+tripulante->pos_x,"%c",tripulante->mapid);
        attroff(COLOR_PAIR(tripulante->pid) | A_BOLD);
        refresh();
    }
    list_iterator_destroy(tripulantes_en_mapa);
    refresh();
    mostrar_patotas_presentes_en_mapa();
    refresh();
    // sleep(1);
}


void iniciar_patota_en_mapa(uint32_t pid, t_list* lista_tcb)
{
    init_pair(1,COLOR_WHITE,COLOR_BLUE);
	init_pair(2,COLOR_BLUE,COLOR_MAGENTA);
	init_pair(3,COLOR_WHITE,COLOR_CYAN);
	init_pair(4,COLOR_CYAN,COLOR_WHITE);
	init_pair(5,COLOR_RED,COLOR_BLUE);
	init_pair(6,COLOR_RED,COLOR_BLACK);
	init_pair(7,COLOR_YELLOW,COLOR_BLACK);
    // nivel_gui_dibujar(AmongOS);


    // attrset(COLOR_PAIR(pid)  | A_BOLD);
    // attron(COLOR_PAIR(pid) | A_BOLD);
    // printw("Patota %d",pid);
    // nivel_gui_dibujar(AmongOS);
    // refresh();

	void agregar_a_mapa(t_tcb* tcb){
			
            // pthread_mutex_lock(&mutex_mapa);    
			t_mapa dto;
			dto.pid = pid;
			dto.tid = tcb->tid;
			dto.pos_x = tcb->posicion_x;
			dto.pos_y = tcb->posicion_y;
			dto.mapid = ID_MAPA;
            ID_MAPA ++;
            // pthread_mutex_unlock(&mutex_mapa);    
			
			void* aux;
			aux = malloc(sizeof(t_mapa));

			memcpy(aux, &dto, sizeof(t_mapa));

			list_add(TABLA_DE_MAPA, aux);
            // refrescar_tabla_de_mapas();
			// crear_personaje(dto.mapid,dto.pos_x,dto.pos_y);
	}

	list_iterate(lista_tcb, agregar_a_mapa);
    // list_sort(TABLA_DE_MAPA,orden_lista_mapa);
    refrescar_tabla_de_mapas();
    // mvprintw(y_max-2,21+pid,"%d",pid);
    // attroff(COLOR_PAIR(pid) | A_BOLD);
    // mostrar_patotas_presentes_en_mapa();
    // refresh();
}

char obtener_id_mapa(uint32_t pid, uint32_t tid)
{
	t_mapa* aux;
	bool buscar_id_mapa(t_mapa* personaje){
		return (personaje->pid == pid && personaje->tid == tid);
	}
	aux = list_find(TABLA_DE_MAPA, buscar_id_mapa);
	return aux->mapid;
}


/* ---------------------------FIN MAP--------------------------------- */

/* ----------------------Administrar Memoria-------------------------- */

void reservar_espacio_de_memoria()
{
    SMOBJ_SIZE = config_get_int_value(config,"TAMANIO_MEMORIA");
    MEMORIA = (void*) malloc (SMOBJ_SIZE);

};

void liberar_espacio_de_memoria()
{
    list_iterate(admin_segmentacion->segmentos_libres,iterator_segmentos_free);
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

        if (anteultimo->fin+1 == ultimo->inicio)
        {
            ultimo->inicio -= diferencia_anteultimo;
            anteultimo = list_remove(admin_segmentacion->segmentos_libres,list_size(admin_segmentacion->segmentos_libres)-2);
            free(anteultimo);
        }

        
    }

}

t_list* segmentos_ocupados_afectados(t_list* lista_ocupados)
{
    t_list* lista_ocupados_filtrados;

    lista_ocupados_filtrados = list_filter(lista_ocupados, condicion_segmento_afectado);
    list_sort(lista_ocupados_filtrados,orden_lista_segmentos);
    return lista_ocupados_filtrados;
}


/* --------------------Fin Administrar Memoria------------------------ */

/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */

/* --------------------------Segmentacion----------------------------- */

void iniciar_segmentacion()
{
    admin_segmentacion = malloc(sizeof(administrador_de_segmentacion));
    t_list* segmentos_libre = list_create();
    
    // Aca hace caca cuando quiero imprimir el archivo de dump
    t_segmento* segmento = malloc(sizeof(t_segmento));

    segmento->inicio = (uint32_t) 0;
    segmento->fin = (uint32_t) config_get_int_value(config,"TAMANIO_MEMORIA");
    segmento->se_encuentra = 0;
    segmento->tipo = 'L';

    list_add(segmentos_libre,segmento);
    admin_segmentacion->bytes_libres = (uint32_t) config_get_int_value(config,"TAMANIO_MEMORIA");
    admin_segmentacion->segmentos_libres = segmentos_libre;

    CRITERIO = config_get_string_value(config,"CRITERIO");

    limpiar_memoria();

    lista_de_patotas = list_create();
    TABLA_DE_MAPA = list_create();
    list_dump_segmentacion = list_create();

    pthread_mutex_init(&mutex_segmentos_libres,NULL);


};


void asignar_segmento(uint32_t bytes_ocupados)
{
    t_segmento* segmento = buscar_segmento_libre(bytes_ocupados);


    free(segmento);
};

t_segmento* buscar_segmento_libre(uint32_t bytes_ocupados)
{
    t_segmento* segmento_libre_buscado;
    if ( strcmp(CRITERIO,"FIRSTFIT") == 0)
    {
        segmento_libre_buscado = buscar_segmento_libre_first_fit(bytes_ocupados);
    }else
    {
        segmento_libre_buscado = buscar_segmento_libre_best_fit(bytes_ocupados);
    }

    return segmento_libre_buscado;
}




t_segmento* buscar_segmento_libre_first_fit(uint32_t bytes_ocupados)
{
    // pthread_mutex_lock(&mutex_segmentos_libres);

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
                segmento_ocupado->inicio = (uint32_t) segmento_libre->inicio;
                segmento_ocupado->fin = (uint32_t) segmento_libre->inicio + (uint32_t) bytes_ocupados - 1;

                segmento_libre->inicio += bytes_ocupados;
                
                
            }else
            {
                segmento_ocupado = list_remove(admin_segmentacion->segmentos_libres,list_iterator->index);
            }
            encontrado = true;
        }
    }

    free(list_iterator);

    if (encontrado)
    {
        segmento_ocupado->se_encuentra = 1;
        // pthread_mutex_unlock(&mutex_segmentos_libres);
        return segmento_ocupado;
        
    }
    else{
        compactar();
        segmento_ocupado = buscar_segmento_libre_first_fit(bytes_ocupados);
        // log_info(logger,"No hay espacio para colocar esta cantidad de bytes, te recomendaria compactar");
        // pthread_mutex_unlock(&mutex_segmentos_libres);
        return segmento_ocupado;
    }
}

t_segmento* buscar_segmento_libre_best_fit(uint32_t bytes_ocupados)
{
    // pthread_mutex_lock(&mutex_segmentos_libres);
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
        }
        if(bytes_ocupados == diferencia)
        {
            segmento_ocupado = list_remove(admin_segmentacion->segmentos_libres,list_iterator->index);
            encontrado = true;
        }
    }
    free(list_iterator);
    
    list_sort(admin_segmentacion->segmentos_libres,orden_lista_admin_segmentacion);
    

    // pthread_mutex_unlock(&mutex_segmentos_libres);
    segmento_ocupado->se_encuentra = 1;

    return segmento_ocupado;
}

bool es_necesario_compactar(int tamanio_tareas, int cantidad_de_tripulantes)
{
    t_list* lista_a_ocupar = list_create();
    list_add(lista_a_ocupar,tamanio_tareas);
    for (int i = 1; i <= cantidad_de_tripulantes; i++)
    {
        list_add(lista_a_ocupar,sizeof(t_tcb));
    }
    list_add(lista_a_ocupar,sizeof(t_pcb));
    list_sort(lista_a_ocupar,orden_mayor_a_menor);
    int tamanio_lista_a_ocupar_original = list_size(lista_a_ocupar);

    // pthread_mutex_lock(&mutex_segmentos_libres);
    list_sort(admin_segmentacion->segmentos_libres,orden_lista_admin_segmentacion_best_free_mayor_a_menor);
    t_list_iterator* list_iterator = list_iterator_create(admin_segmentacion->segmentos_libres);
    bool encontrados = false;
    
    t_segmento* segmento_libre;
    int valor_a_guardar;
    int indice = 0;
    int contador_de_eliminados = 0;
    while(!encontrados && list_iterator_has_next(list_iterator))
    {
        segmento_libre = (t_segmento*) list_iterator_next(list_iterator);
        uint32_t diferencia = (segmento_libre->fin - segmento_libre->inicio)+1;
        valor_a_guardar = list_remove(lista_a_ocupar,indice);
        contador_de_eliminados ++;
        if (valor_a_guardar > diferencia)
        {
            list_destroy(lista_a_ocupar);
            free(list_iterator);
            list_sort(admin_segmentacion->segmentos_libres,orden_lista_admin_segmentacion_best_free);
            return true;
        }
        int nuevo_valor_libre;
        while (valor_a_guardar < diferencia && !encontrados)
        {
            nuevo_valor_libre = diferencia - valor_a_guardar;
            valor_a_guardar = list_get(lista_a_ocupar,indice+1);
            
            if (valor_a_guardar <= nuevo_valor_libre)
            {
                contador_de_eliminados ++;
                valor_a_guardar = list_remove(lista_a_ocupar,indice);
                diferencia = nuevo_valor_libre;
                // indice == list_size(admin_segmentacion->segmentos_libres)
                if ( contador_de_eliminados+1 == tamanio_lista_a_ocupar_original )
                {
                    encontrados = true;
                }
                
            }
            
        }
    list_destroy(lista_a_ocupar);
    free(list_iterator);
    list_sort(admin_segmentacion->segmentos_libres,orden_lista_admin_segmentacion_best_free);

    return false;
    }
}



t_segmento* buscar_segmento_libre_best_fit_original(uint32_t bytes_ocupados)
{
    t_list_iterator* list_iterator = list_iterator_create(admin_segmentacion->segmentos_libres);

    t_segmento* segmento_libre;
    uint32_t menor_distancia = SMOBJ_SIZE;
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
    // pthread_mutex_lock(&sem_memoria);    
        int desplazamiento = segmento_a_ocupar->inicio;
    
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
    // pthread_mutex_unlock(&sem_memoria); 
    
    desplazamiento += sizeof(uint32_t)-1;

    if (desplazamiento != segmento_a_ocupar->fin)
    {
        log_info(logger,"Un tcb se cargo mal a memoria");
    }
}

void actualizar_en_MEMORIA_tcb_prox_tarea(t_segmento* segmento_a_modificar)
{
        // pthread_mutex_lock(&sem_memoria); 
        int desplazamiento_proxima_tarea = segmento_a_modificar->inicio;
        desplazamiento_proxima_tarea += sizeof(uint32_t)+sizeof(char)+sizeof(uint32_t)+sizeof(uint32_t);
        char* MEMORIA_prox_tarea = MEMORIA;
        uint32_t prox_tarea_aux = MEMORIA_prox_tarea[desplazamiento_proxima_tarea];
        prox_tarea_aux += 1;
        memcpy(MEMORIA + desplazamiento_proxima_tarea, (&prox_tarea_aux), sizeof(uint32_t));
        // pthread_mutex_unlock(&sem_memoria); 
}

void actualizar_en_MEMORIA_tcb_estado(t_segmento* segmento_a_modificar, char estado)
{
        // pthread_mutex_lock(&sem_memoria); 
        int desplazamiento_estado = segmento_a_modificar->inicio;
        desplazamiento_estado += sizeof(uint32_t);

        memcpy(MEMORIA + desplazamiento_estado, (&estado), sizeof(char));
        // pthread_mutex_unlock(&sem_memoria); 
}

void actualizar_en_MEMORIA_tcb_posiciones(t_segmento* segmento_a_modificar, uint32_t pos_x, uint32_t pos_y)
{
        // pthread_mutex_lock(&sem_memoria); 
        int desplazamiento_posiciones = segmento_a_modificar->inicio;
        desplazamiento_posiciones += sizeof(uint32_t) +sizeof(char);

        memcpy(MEMORIA + desplazamiento_posiciones, (&pos_x), sizeof(uint32_t));
        desplazamiento_posiciones += sizeof(uint32_t);
        memcpy(MEMORIA + desplazamiento_posiciones, (&pos_y), sizeof(uint32_t));
        // pthread_mutex_unlock(&sem_memoria); 
}


void guardar_en_MEMORIA_pcb(t_segmento* segmento_a_ocupar,t_pcb* pcb)
{
        int desplazamiento = segmento_a_ocupar->inicio;
    // pthread_mutex_lock(&sem_memoria); 
    memcpy(MEMORIA + desplazamiento, (&pcb->pid), sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(MEMORIA + desplazamiento, (&pcb->tareas), sizeof(uint32_t));
    // pthread_mutex_unlock(&sem_memoria); 
    desplazamiento += sizeof(uint32_t)-1;
    if (desplazamiento != segmento_a_ocupar->fin)
    {
        log_info(logger,"Un pcb se cargo mal a memoria");
    }

}

void guardar_en_MEMORIA_tareas(t_segmento* segmento_a_ocupar,char* tareas_unidas)
{
    int desplazamiento = segmento_a_ocupar->inicio;
    // loggear_linea();
    // loggear_entero(segmento_a_ocupar->inicio);
    // loggear_entero(segmento_a_ocupar->fin);
    // loggear_linea();
    // pthread_mutex_lock(&sem_memoria); 
    memcpy(MEMORIA + desplazamiento, (void*) (tareas_unidas), strlen(tareas_unidas));
    // pthread_mutex_unlock(&sem_memoria); 
    int tamanio = strlen(tareas_unidas);
    desplazamiento += strlen(tareas_unidas);
    if (desplazamiento != segmento_a_ocupar->fin+1)
    {
        log_info(logger,"Las tareas se cargaron mal a memoria");
    }
}




void liberar_segmento(t_segmento* segmento)
{
    // pthread_mutex_lock(&mutex_segmentos_libres);

    admin_segmentacion->bytes_libres += segmento->fin - segmento->inicio;
    
    for (int i = segmento->inicio; i <= segmento->fin; i++)
    {
        // pthread_mutex_lock(&sem_memoria); 
        memcpy(MEMORIA+i, (void*) " ", sizeof(char));
        // pthread_mutex_unlock(&sem_memoria); 
    }
    
    segmento->se_encuentra = 0;
    segmento->tipo = 'L';
    list_add(admin_segmentacion->segmentos_libres, segmento);
    
    list_sort(admin_segmentacion->segmentos_libres,orden_lista_admin_segmentacion);
    // pthread_mutex_unlock(&mutex_segmentos_libres);
}

void compactar()
{
    // pthread_mutex_lock(&mutex_segmentos_libres);
    t_list* lista_ocupada = lista_de_segmentos_ocupados();
        
            
    t_list* segmentos_afectados;
    t_list* segmentos_transformados;
    list_sort(admin_segmentacion->segmentos_libres,orden_lista_admin_segmentacion);


    while (list_size(admin_segmentacion->segmentos_libres)>1 && ultimo_segmento_libre_compactable()->fin != SMOBJ_SIZE)
    {
        consolidar_segmentos_libres_se_es_posbile();
        segmentos_afectados = segmentos_ocupados_afectados(lista_ocupada);
        segmentos_transformados = list_map(segmentos_afectados, transformacion_segmento_afectado);
        // list_iterate(segmentos_afectados, (void*) iterator_segmento);
        list_destroy(segmentos_afectados);
        list_destroy(segmentos_transformados);
    }

    list_sort(admin_segmentacion->segmentos_libres,orden_lista_admin_segmentacion_best_free);
    list_destroy(lista_ocupada);
    // pthread_mutex_unlock(&mutex_segmentos_libres);

}

void colocar_ultimo_segmento_libre_al_final()
{
    if (ultimo_segmento_libre()->fin != SMOBJ_SIZE)
    {
        int diferencia = ultimo_segmento_libre()->fin - ultimo_segmento_libre()->inicio;
        ultimo_segmento_libre()->fin = SMOBJ_SIZE;
        ultimo_segmento_libre()->inicio = SMOBJ_SIZE - diferencia;
    }

}

t_list* lista_de_segmentos_ocupados()
{
    t_list* lista_ocupados = list_create();
    t_list* lista_patota_filtrados, *lista_patota;
    t_list_iterator* list_iterator_patotas = list_iterator_create(lista_de_patotas);
    while(list_iterator_has_next(list_iterator_patotas))
    {
        lista_patota = (t_segmento*) list_iterator_next(list_iterator_patotas);
        
        lista_patota_filtrados = list_filter(lista_patota, condicion_segmento_presente_en_memoria);
        list_add_all(lista_ocupados,lista_patota_filtrados);

        list_clean(lista_patota_filtrados);
        list_destroy(lista_patota_filtrados);
    }
    free(list_iterator_patotas);

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

    log_info(logger,"\tEl primer byte despues es: %d",ultimo_segmento_libre->inicio);
    log_info(logger,"\tEl ultimo byte despues es: %d",ultimo_segmento_libre->fin);

}

void liberar_patota_si_no_hay_tripulantes(int pid)
{
    t_list* segmentos_patota;
    segmentos_patota = retornar_segmentos_patota(pid);
    bool hay_tripulantes = false;
    t_segmento* segmento_tcb;

    for (int i = 1; i < list_size(segmentos_patota)-1; i++)
    {
        segmento_tcb = list_get(segmentos_patota,i);
        if (segmento_tcb->se_encuentra == 1)
        {
            hay_tripulantes = true;
        }
        
    }

    if (!hay_tripulantes)
    {
        t_segmento* segmento_a_liberar;
        segmento_a_liberar = list_get(segmentos_patota,0);
        liberar_segmento(segmento_a_liberar);
        segmento_a_liberar = list_get(segmentos_patota,list_size(segmentos_patota)-1);
        liberar_segmento(segmento_a_liberar);

        for (int i = 1; i < list_size(segmentos_patota)-1; i++)
        {
            segmento_tcb = list_get(segmentos_patota,i);
            free(segmento_tcb);
        }
        // for (int i = 0; i < list_size(segmentos_patota); i++)
        // {
        //     segmento_a_liberar = list_get(segmentos_patota,i);
        //     if (segmento_a_liberar->se_encuentra == 1)
        //     {
        //         liberar_segmento(segmento_a_liberar);
        //     }
        // }
    }
    
    
}
void iniciar_patota_SEGMENTACION (t_list* lista,t_pcb* pcb,t_list* tcbs, char* tareas_unidas, int cant_tripulantes)
{
/* ------------------------Asignacion de segmentos---------------------------- */
    /* ---------------------validar espacio disponible-------------------------- */
        int _tareas_bytes_ocupados = strlen(tareas_unidas);
        int bytes_a_guardar = _tareas_bytes_ocupados + sizeof(t_pcb) + cant_tripulantes * sizeof(t_tcb);
        
        // loggear_linea();
        // log_info(logger, "Es necsario compactar? -> %s",es_necesario_compactar(_tareas_bytes_ocupados,cant_tripulantes)?"true":"false");
        
        // Se pregunta si hay espacio para TODAS los segmentos
        pthread_mutex_lock(&mutex_segmentos_libres);
        pthread_mutex_lock(&sem_memoria); 

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
            }
            pthread_mutex_lock(&mutex_segmentos_libres);
            pthread_mutex_lock(&sem_memoria); 
            pthread_mutex_lock(&mutex_mapa);
            compactar();
            pthread_mutex_unlock(&mutex_segmentos_libres);
            pthread_mutex_unlock(&sem_memoria); 
            pthread_mutex_unlock(&mutex_mapa);
        }
        // if ( strcmp(CRITERIO,"BESTFIT") == 0 && es_necesario_compactar(_tareas_bytes_ocupados,cant_tripulantes))
        if ( es_necesario_compactar(_tareas_bytes_ocupados,cant_tripulantes) )
        {
            pthread_mutex_lock(&mutex_segmentos_libres);
            pthread_mutex_lock(&sem_memoria); 
            pthread_mutex_lock(&mutex_mapa);
            compactar();
            pthread_mutex_unlock(&mutex_segmentos_libres);
            pthread_mutex_unlock(&sem_memoria); 
            pthread_mutex_unlock(&mutex_mapa);
        }
        
        // pthread_mutex_unlock(&mutex_segmentos_libres);
        
        t_list* tabla_segmentos_de_patota = (t_segmento*) list_create();

    /* -----------------------------Tareas-------------------------------------- */
        t_segmento* segmento_tareas;
        segmento_tareas = buscar_segmento_libre(_tareas_bytes_ocupados);
        segmento_tareas->tipo = 'I';
        // pthread_mutex_lock(&sem_memoria); 
        guardar_en_MEMORIA_tareas(segmento_tareas,tareas_unidas);
        // pthread_mutex_unlock(&sem_memoria); 
    /* ------------------------------PCB---------------------------------------- */
        t_segmento* segmento_pcb;
        segmento_pcb = buscar_segmento_libre(sizeof(t_pcb));
        segmento_pcb->tipo = 'P';
        
        pcb->tareas = segmento_tareas->inicio;
        guardar_en_MEMORIA_pcb(segmento_pcb,pcb);
        
        list_add(tabla_segmentos_de_patota,segmento_pcb);
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
            
        }
        free(list_iterator);

    /*----------------------Fin Asignacion de segmentos--------------------------*/
    /*-----------------------Hidratacion de la patota----------------------------*/

    // Esto tendra el orden de PCB, TCB1 .. TCB N, tareas (en la posicion N de la lista)
        list_add(tabla_segmentos_de_patota,segmento_tareas);

        list_add_sorted(lista_de_patotas,tabla_segmentos_de_patota,comparador_patotas);
        // list_add(lista_de_patotas,tabla_segmentos_de_patota);
        // list_sort(lista_de_patotas,comparador_patotas);
        pthread_mutex_unlock(&mutex_segmentos_libres);
        pthread_mutex_unlock(&sem_memoria); 
        
    /*---------------------Fin Hidratacion de la patota--------------------------*/
}

char* enviar_proxima_tarea_SEGMENTACION(uint32_t pid, uint32_t tid)
{
    pthread_mutex_lock(&mutex_segmentos_libres);
    pthread_mutex_lock(&sem_memoria); 

    //  lista de segmentos de ese pid de las estructuras administrativas
    t_list* segmentos_patota;
    segmentos_patota = retornar_segmentos_patota(pid);
    
    // Obtenemos el segmento de TAREAS de las estructuras administrativas
    t_segmento* segmento_tareas_prox_tarea;
    segmento_tareas_prox_tarea = retornar_segmento_tareas(segmentos_patota);
    // Obtenemos el segmento de TCB ESPECIFICO (TID) las estructuras administrativas
    t_segmento* segmento_tcb_prox_tarea;
    segmento_tcb_prox_tarea = retornar_segmento_tcb(segmentos_patota,tid);

    // Entero que representa la proxima tarea del tripulante, se actualiza despues de devolver
    int prox_tarea = retornar_prox_inst_del_tcb(segmento_tcb_prox_tarea);

    // se calcula el tamanio de la tareas unidas
    int tamanio_tarea = segmento_tareas_prox_tarea->fin - segmento_tareas_prox_tarea->inicio;

    // Obtenemos las TAREAS que se encuentran en MEMORIA
    char* tareas_de_segmentos_patota;
    tareas_de_segmentos_patota = retornar_tareas(segmento_tareas_prox_tarea);
        // Descomentar para testear
    // log_info(logger,"El taeritas es: %s",tareas_de_segmentos_patota);

    // Obtenemos las TAREA ESPECIFICA (la proxima) que se encuentran en MEMORIA
    char* tarea_solicitada;
    tarea_solicitada = retornar_tarea_solicitada(tareas_de_segmentos_patota,prox_tarea);


    // liberar estructuras utilizadas

    pthread_mutex_unlock(&mutex_segmentos_libres);
    pthread_mutex_unlock(&sem_memoria); 
    free(tareas_de_segmentos_patota);
    return tarea_solicitada;
}


void actualizar_estado_SEGMENTACION (uint32_t pid_actualizar_estado, uint32_t tid_actualizar_estado, char estado_actualizar_estado)
{
    pthread_mutex_lock(&mutex_segmentos_libres);
    pthread_mutex_lock(&sem_memoria); 

    //  lista de segmentos de ese pid de las estructuras administrativas
    t_list* segmentos_patota_actualizar_estado;
    segmentos_patota_actualizar_estado = retornar_segmentos_patota(pid_actualizar_estado);
    
    // Obtenemos el segmento de TCB ESPECIFICO (TID) las estructuras administrativas
    t_segmento* segmento_tcb_actualizar_estado;
    segmento_tcb_actualizar_estado = retornar_segmento_tcb(segmentos_patota_actualizar_estado,tid_actualizar_estado);

    // Actualizar el estado del tripulante
    actualizar_en_MEMORIA_tcb_estado(segmento_tcb_actualizar_estado,estado_actualizar_estado);
    // loggear_tcb(segmento_tcb_actualizar_estado);
    pthread_mutex_unlock(&mutex_segmentos_libres);
    pthread_mutex_unlock(&sem_memoria); 
}

void expulsar_tripulante_SEGMENTACION(uint32_t pid_expulsar_tripulante, uint32_t tid_expulsar_tripulante)
{
    pthread_mutex_lock(&mutex_segmentos_libres);
    pthread_mutex_lock(&sem_memoria); 

    // lista de segmentos de ese pid de las estructuras administrativas
    t_list* segmentos_patota_expulsar_tripulante;
    segmentos_patota_expulsar_tripulante = retornar_segmentos_patota(pid_expulsar_tripulante);

    // Obtenemos el segmento de TCB de las estructuras administrativas
    t_segmento* segmento_tcb_explulsar_tripulante;
    segmento_tcb_explulsar_tripulante = retornar_segmento_tcb(segmentos_patota_expulsar_tripulante,tid_expulsar_tripulante);
    
    // int pos_x,pos_y;
    // pos_x = retornar_pos_x_del_tcb(segmento_tcb_explulsar_tripulante);
    // pos_y = retornar_pos_y_del_tcb(segmento_tcb_explulsar_tripulante);
    // eliminar_personaje_ubicado(id_mapa,pos_x,pos_y);
    
    t_segmento* segmento_vacio = malloc(sizeof(t_segmento));
    segmento_vacio->se_encuentra = 0;
    list_replace(segmentos_patota_expulsar_tripulante,tid_expulsar_tripulante,segmento_vacio);
    liberar_segmento(segmento_tcb_explulsar_tripulante);
    liberar_patota_si_no_hay_tripulantes(pid_expulsar_tripulante);

    pthread_mutex_unlock(&mutex_segmentos_libres);
    pthread_mutex_unlock(&sem_memoria); 
}

void cambiar_ubicacion_tripulante_SEGMENTACION(uint32_t pid_ubicacion,uint32_t tid_ubicacion,uint32_t nueva_pos_x,uint32_t nueva_pos_y)
{
    pthread_mutex_lock(&mutex_segmentos_libres);
    pthread_mutex_lock(&sem_memoria);

    // lista de segmentos de ese pid de las estructuras administrativas
    t_list* segmentos_patota_ubicacion;
    segmentos_patota_ubicacion = retornar_segmentos_patota(pid_ubicacion);
    t_segmento* segmento_tcb_ubicacion, *segmento_pcb_ubicacion;
    segmento_pcb_ubicacion = list_get(segmentos_patota_ubicacion,0);
    segmento_tcb_ubicacion = retornar_segmento_tcb(segmentos_patota_ubicacion,tid_ubicacion);

        // Descomentar para testear
    // loggear_tcb(segmento_tcb_ubicacion);

    // uint32_t pos_x,pos_y,pid;
    // t_segmento* pcb;
    // pos_x = retornar_pos_x_del_tcb(segmento_tcb_ubicacion);
    // pos_y = retornar_pos_y_del_tcb(segmento_tcb_ubicacion);
    // pid = retornar_pid_del_pcb(segmento_pcb_ubicacion);
    // mover_personaje_ubicacion(pid,id,pos_x,pos_y,nueva_pos_x,nueva_pos_y);
    
    actualizar_en_MEMORIA_tcb_posiciones(segmento_tcb_ubicacion,nueva_pos_x,nueva_pos_y);
    
    pthread_mutex_unlock(&mutex_segmentos_libres);
    pthread_mutex_unlock(&sem_memoria);

        // Descomentar para testear
    // loggear_tcb(segmento_tcb_ubicacion);
}




/* ------------------------Fin Segmentacion--------------------------- */

/* --------------------------PAGINACION----------------------------- */

void iniciar_paginacion()
{   
    char* algoritmo = config_get_string_value(config,"ALGORITMO_REEMPLAZO");
    if ( strcmp(algoritmo,"LRU") == 0)
    {
        MODO_DESALOJO = 0;
    }else
    {
        MODO_DESALOJO = 1;
    }
    
    TAMANIO_MEMORIA_VIRTUAL = config_get_int_value(config,"TAMANIO_MEMORIA_VIRTUAL");
    MEMORIA_VIRTUAL = (void*) malloc (TAMANIO_MEMORIA_VIRTUAL);

    TAMANIO_PAGINAS = config_get_int_value(config,"TAMANIO_PAGINA");
    TAMANIO_MEMORIA = config_get_int_value(config,"TAMANIO_MEMORIA");

    CANTIDAD_MARCOS = obtener_tamanio_array_de_marcos();
	ESTADO_MARCOS = (uint32_t *) malloc( sizeof (uint32_t) * CANTIDAD_MARCOS);
	TIMESTAMP_MARCOS = (uint32_t *) malloc( sizeof (uint32_t) * CANTIDAD_MARCOS);
	ARRAY_BIT_USO =  (uint32_t *) malloc( sizeof (uint32_t) * CANTIDAD_MARCOS);
	CANTIDAD_MARCOS_VIRTUALES = obtener_tamanio_array_de_marcos_virtuales();
	ESTADO_MARCOS_VIRTUALES = (uint32_t *) malloc( sizeof (uint32_t) * CANTIDAD_MARCOS_VIRTUALES);
	
	//inicializo en 0 todo el bitmap

	for (int i = 0; i < CANTIDAD_MARCOS; i++) {
		ESTADO_MARCOS[i] = 0;
		TIMESTAMP_MARCOS[i] = 0;
		ARRAY_BIT_USO[i] = 0;
	}

	for (int i = 0; i < CANTIDAD_MARCOS_VIRTUALES; i++) {
		ESTADO_MARCOS_VIRTUALES[i] = 0;
	}

	TABLA_DE_PAGINAS = list_create();
    pthread_mutex_init(&mutex_paginacion,NULL);


	int fd = open("cfg/virtualmemory.txt",O_RDWR , S_IRUSR | S_IWUSR);
	struct stat sb;

	ftruncate(fd, TAMANIO_MEMORIA_VIRTUAL);

	if(fstat(fd, &sb) == -1) {
		perror("No pude obtener el tamaño del archivo.\n");
	}

	// printf("Tamaño del Archivo: %ld\n",sb.st_size);

	MEMORIA_VIRTUAL = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    TABLA_DE_MAPA = list_create();
}

uint32_t obtener_tamanio_array_de_marcos (){
	if (TAMANIO_MEMORIA >= TAMANIO_PAGINAS) {
		//printf("La cantidad de marcos disponibles es de %d marcos\n", TAMANIO_MEMORIA / TAMANIO_PAGINAS);
		//printf("La cantidad de memoria que quedara inutilizable sera de %d bytes\n", TAMANIO_MEMORIA % TAMANIO_PAGINAS);
		return TAMANIO_MEMORIA/TAMANIO_PAGINAS;
	} else {
		printf("Hubo un error en la asignacion de ESTADO_MARCOS");
		return 10;
	}
}

uint32_t obtener_tamanio_array_de_marcos_virtuales (){
	if (TAMANIO_MEMORIA_VIRTUAL >= TAMANIO_PAGINAS) {
		//printf("La cantidad de marcos disponibles es de %d marcos\n", TAMANIO_MEMORIA / TAMANIO_PAGINAS);
		//printf("La cantidad de memoria que quedara inutilizable sera de %d bytes\n", TAMANIO_MEMORIA % TAMANIO_PAGINAS);
		return TAMANIO_MEMORIA_VIRTUAL/TAMANIO_PAGINAS;
	} else {
		printf("Hubo un error en la asignacion de ESTADO_MARCOS_VIRTUALES");
		return 10;
	}
}

uint32_t redondear_para_arriba (uint32_t numero, uint32_t divisor) {
	if (numero%divisor!= 0){
		return numero/divisor+1;
	} else {
		return numero/divisor;
	}
}

dto_memoria empaquetar_data_paginacion (estructura_administrativa_paginacion* data_a_empaquetar){
	dto_memoria dto_aux;

	uint32_t aux_tamanio = 0;
	uint32_t cantidad_de_tripulantes = list_size(data_a_empaquetar->lista_de_tcb);
	uint32_t largo_lista_de_tareas = strlen(data_a_empaquetar->lista_de_tareas) + 1;
	aux_tamanio += 8;
	aux_tamanio += cantidad_de_tripulantes * 21;
	data_a_empaquetar->pcb.tareas = aux_tamanio;
	aux_tamanio += largo_lista_de_tareas;
	
	dto_aux.tamanio_data = aux_tamanio;
	void* memaux;
	memaux = (void*) malloc (redondear_para_arriba(aux_tamanio, TAMANIO_PAGINAS)*TAMANIO_PAGINAS);
	memcpy(memaux, &data_a_empaquetar->pcb, 8);
	for (uint32_t i = 0; i < cantidad_de_tripulantes; i++ ) {
		 t_tcb* aux_tcb = list_get(data_a_empaquetar->lista_de_tcb, i);
		 memcpy(memaux + 8 + 21*i, &aux_tcb->tid, 4);
		 memcpy(memaux + 12 + 21*i, &aux_tcb->estado, 1);
		 memcpy(memaux + 13 + 21*i, &aux_tcb->posicion_x, 4);
		 memcpy(memaux + 17 + 21*i, &aux_tcb->posicion_y, 4);
		 memcpy(memaux + 21 + 21*i, &aux_tcb->proxima_instruccion, 4);
		 memcpy(memaux + 25 + 21*i, &aux_tcb->puntero_pcb, 4);
	}
	memcpy(memaux + 8 + cantidad_de_tripulantes * 21, data_a_empaquetar->lista_de_tareas, largo_lista_de_tareas);
	dto_aux.data_empaquetada = memaux;
	
	return dto_aux;
}

void modificar_tlb (uint32_t id_proceso, t_list* lista_tids, t_list* marcos_utilizados) {
	
	bool find(void* element){
		t_tabla_proceso* aux = element;
		return aux->pid == id_proceso;
	}

		list_remove_and_destroy_by_condition(TABLA_DE_PAGINAS, find, free);
		t_tabla_proceso* dto = malloc(sizeof(t_tabla_proceso));
		dto->pid = id_proceso;
		dto->lista_de_marcos = marcos_utilizados;
		dto->lista_de_tids = lista_tids;
		t_list* lista_presencia;
		lista_presencia = list_create();
		uint32_t aux2 = list_size(marcos_utilizados);
		for(uint32_t i = 0; i<aux2; i++) {
			list_add(lista_presencia, 1);
		}	
		dto->lista_de_presencia = lista_presencia;
		list_add(TABLA_DE_PAGINAS, dto);
}

uint32_t obtener_marcos_vacios() {
	uint32_t marcos_vacios = 0;
	
	for(uint32_t i=0; i<CANTIDAD_MARCOS; i++) {
		if(ESTADO_MARCOS[i]==0) {
			marcos_vacios++;
		}
	} 
	return marcos_vacios;
}

uint32_t obtener_marcos_vacios_virtuales() {
	uint32_t marcos_vacios_virtuales = 0;
	
	for(uint32_t i=0; i<CANTIDAD_MARCOS_VIRTUALES; i++) {
		if(ESTADO_MARCOS_VIRTUALES[i]==0) {
			marcos_vacios_virtuales++;
		}
	} 
	return marcos_vacios_virtuales;
}

uint32_t paginas_que_ocupa(uint32_t cantidad_bytes){
	uint32_t aux = cantidad_bytes / TAMANIO_PAGINAS;
	if (cantidad_bytes % TAMANIO_PAGINAS != 0) {
		aux ++;
	}
	return aux;
}

void setear_memoria(t_list* lista_a_reservar, void* data_empaquetada){
	uint32_t aux = 0;
	void setear_una_pagina(uint32_t numero_marco){
		memcpy(MEMORIA + (numero_marco * TAMANIO_PAGINAS), data_empaquetada + aux, TAMANIO_PAGINAS);
		aux = TAMANIO_PAGINAS + aux;
	}
	list_iterate(lista_a_reservar, (void*)setear_una_pagina);
	free(data_empaquetada);
}

void pasar_un_marco_de_memoria(uint32_t marco_virtual, uint32_t marco_principal, bool sentido){
	if(sentido==false){	
		memcpy(MEMORIA_VIRTUAL + (marco_virtual*TAMANIO_PAGINAS), MEMORIA +(marco_principal*TAMANIO_PAGINAS), TAMANIO_PAGINAS);
	} else {
		memcpy(MEMORIA + (marco_principal*TAMANIO_PAGINAS), MEMORIA_VIRTUAL + (marco_virtual*TAMANIO_PAGINAS), TAMANIO_PAGINAS);
	}
}


t_list* obtener_marcos_a_reservar(uint32_t paginas_solicitadas){
	t_list* aux_list = list_create();
	uint32_t contador_array = 0;
	for(uint32_t i=0; i<paginas_solicitadas; i++) {
		if(ESTADO_MARCOS[contador_array]==0) {
			list_add(aux_list, contador_array);
		} else {
			i--;
		}
		contador_array++;
	} 
	return aux_list;
}

void setear_marco_como_usado(uint32_t numero_marco){
		ESTADO_MARCOS[numero_marco] = 1;
		TIMESTAMP_MARCOS[numero_marco] = COUNTER_LRU;
		COUNTER_LRU++;
		ARRAY_BIT_USO[numero_marco] = 1;
}

void setear_marcos_usados(t_list* lista_a_reservar){
	list_iterate(lista_a_reservar, (void*)setear_marco_como_usado);
}

uint32_t indice_del_minimo_de_un_array(uint32_t* array, uint32_t tamanio_array){
	uint32_t min = array[0];
	uint32_t indice_min = 0;
	for(uint32_t i = 1; i<tamanio_array; i++) {
		if(min > array[i]){
			min = array[i];
			indice_min = i;
		}
	}
	return indice_min;
}



uint32_t obtener_proceso_de_marco(uint32_t marco){
	t_tabla_proceso* aux;
	bool find(void* element){
		t_tabla_proceso* aux = element;
		bool find2(void* num){
			return num == marco;
		}
		return list_any_satisfy(aux->lista_de_marcos, find2);
	}
	aux = list_find(TABLA_DE_PAGINAS, find);
	return aux->pid;
}

void reemplazar_marco_de_tabla_de_paginas(uint32_t marco_a_reemplazar, uint32_t nuevo_marco, uint32_t presencia) {
	t_tabla_proceso* aux2;
	uint32_t indice = 0;
	uint32_t aux3 = 0;

	bool find(void* element){
		t_tabla_proceso* aux = element;
		bool find2(void* num){
			return num == marco_a_reemplazar;
		}
		return list_any_satisfy(aux->lista_de_marcos, find2);
	}

	void obtener_indice_marco(uint32_t num){	
		if(num == marco_a_reemplazar){
			indice = aux3;
		}else{
			aux3++;
		}
	}

	aux2 = list_find(TABLA_DE_PAGINAS, find);
	list_iterate(aux2->lista_de_marcos, obtener_indice_marco);
	list_replace(aux2->lista_de_marcos, indice, nuevo_marco);
	switch (presencia){
	case 0:
		list_replace(aux2->lista_de_presencia, indice, 0);
		break;

	case 1:
		list_replace(aux2->lista_de_presencia, indice, 1);
		break;
	}
}

uint32_t obtener_marco_virtual_vacio(){
	uint32_t aux = 0;
	while (ESTADO_MARCOS_VIRTUALES[aux]!=0){
		aux++;
	}
	return aux;
}

void desalojar_un_marco(uint32_t marco_a_desalojar){
	if(ESTADO_MARCOS[marco_a_desalojar]==1){
		uint32_t marco_virtual = obtener_marco_virtual_vacio();
		pasar_un_marco_de_memoria(marco_virtual, marco_a_desalojar, false);
		ESTADO_MARCOS[marco_a_desalojar] = 0;
		ESTADO_MARCOS_VIRTUALES[marco_virtual] = 1;
		reemplazar_marco_de_tabla_de_paginas(marco_a_desalojar, marco_virtual + OFFSET,  0);
	}
}

t_list* obtener_marcos_a_desalojar(uint32_t numero_de_paginas_a_desalojar) {
	t_list* victimas = list_create();
	if(MODO_DESALOJO == 0){
		for(uint32_t i=0; i<numero_de_paginas_a_desalojar; i++){
			uint32_t marco_a_desalojar = indice_del_minimo_de_un_array(TIMESTAMP_MARCOS, CANTIDAD_MARCOS);
			TIMESTAMP_MARCOS[marco_a_desalojar] = COUNTER_LRU;
			COUNTER_LRU++;
			list_add(victimas, marco_a_desalojar);
		}
	} else {
		for(uint32_t i=0; i<numero_de_paginas_a_desalojar; i++){
			while (ARRAY_BIT_USO[PUNTERO_CLOCK]!= 0){
				ARRAY_BIT_USO[PUNTERO_CLOCK] = 0;
				PUNTERO_CLOCK ++;

				if (PUNTERO_CLOCK == CANTIDAD_MARCOS){
					PUNTERO_CLOCK = 0;
				} 
			}
			ARRAY_BIT_USO[PUNTERO_CLOCK] = 1;
			list_add(victimas, PUNTERO_CLOCK);
			PUNTERO_CLOCK++;
			if (PUNTERO_CLOCK == CANTIDAD_MARCOS){
					PUNTERO_CLOCK = 0;
			} 
		}
	}
	return victimas;
}

void desalojar(uint32_t numero_de_paginas_a_desalojar) {
	if(numero_de_paginas_a_desalojar!=0){
		t_list* marcos_a_desalojar;
		marcos_a_desalojar = obtener_marcos_a_desalojar(numero_de_paginas_a_desalojar);
		list_iterate(marcos_a_desalojar, desalojar_un_marco);
		list_destroy(marcos_a_desalojar);
	}
}


t_list* listar_tids(estructura_administrativa_paginacion* dato){
	int32_t obtener_tid(t_tcb* elem){
		return elem->tid;	
	}
	return list_map(dato->lista_de_tcb, obtener_tid);
}


void iniciar_patota_PAGINACION (estructura_administrativa_paginacion* dato_a_paginar){
	
    pthread_mutex_lock(&sem_memoria);

    dto_memoria dato_empaquetado = empaquetar_data_paginacion(dato_a_paginar);
	if(paginas_que_ocupa(dato_empaquetado.tamanio_data) <= obtener_marcos_vacios()){
		t_list* marcos_a_reservar = obtener_marcos_a_reservar(paginas_que_ocupa(dato_empaquetado.tamanio_data));
		modificar_tlb(dato_a_paginar->pcb.pid, listar_tids(dato_a_paginar), marcos_a_reservar);
		setear_marcos_usados(marcos_a_reservar);
		setear_memoria(marcos_a_reservar, dato_empaquetado.data_empaquetada);
	
	} else {
		if(paginas_que_ocupa(dato_empaquetado.tamanio_data) <= (obtener_marcos_vacios() + obtener_marcos_vacios_virtuales())) {
			desalojar(paginas_que_ocupa(dato_empaquetado.tamanio_data));
			t_list* marcos_a_reservar = obtener_marcos_a_reservar(paginas_que_ocupa(dato_empaquetado.tamanio_data));
			modificar_tlb(dato_a_paginar->pcb.pid, listar_tids(dato_a_paginar), marcos_a_reservar);
			setear_marcos_usados(marcos_a_reservar);
			setear_memoria(marcos_a_reservar, dato_empaquetado.data_empaquetada);
		}
	}

    pthread_mutex_unlock(&sem_memoria);
}



t_tabla_proceso* buscar_proceso(uint32_t id_proceso){
	bool buscarIdProceso(t_tabla_proceso* elem){
		return elem->pid == id_proceso;
	}

	return list_find(TABLA_DE_PAGINAS, buscarIdProceso);
}

uint32_t indice_de_tripulante(t_list* lista_de_ids, uint32_t id_tripulante){
	uint32_t indice = 0;
	uint32_t aux = 0;

	void buscarIndice(uint32_t elem){
		if(elem == id_tripulante){
			indice = aux;
		}
		aux++;
	}

	list_iterate(lista_de_ids, buscarIndice);
	return indice;
}

t_list* lista_marcos_en_virtual(t_list* lista_de_paginas, uint32_t id_proceso){
	t_tabla_proceso* aux = buscar_proceso(id_proceso);
	t_list* lista_de_marcos = list_create();
	uint32_t mapeo_a_marco_virtual(uint32_t pagina){
		if(0 == list_get(aux->lista_de_presencia, pagina)){
		uint32_t marco = list_get(aux->lista_de_marcos, pagina);
		list_add(lista_de_marcos, marco);
		}
	}
	list_iterate(lista_de_paginas, mapeo_a_marco_virtual);
	return lista_de_marcos;
}

void setear_nueva_posicion(t_list* marcos_a_cambiar, uint32_t posx, uint32_t posy, uint32_t desplazamiento){
	void* memoria_aux;
	uint32_t tamanio_a_alojar = list_size(marcos_a_cambiar)*TAMANIO_PAGINAS;
	uint32_t primer_marco = list_get(marcos_a_cambiar, 0);
	uint32_t ultimo_marco = list_get(marcos_a_cambiar, list_size(marcos_a_cambiar) - 1);

	memoria_aux = (void*) malloc (tamanio_a_alojar);
	memcpy(memoria_aux, MEMORIA + primer_marco * TAMANIO_PAGINAS, desplazamiento);
	memcpy(memoria_aux + desplazamiento, &posx, 4);
	memcpy(memoria_aux + desplazamiento + 4, &posy, 4);
	memcpy(memoria_aux + desplazamiento + 8, MEMORIA + ultimo_marco*TAMANIO_PAGINAS + (desplazamiento + 8)%TAMANIO_PAGINAS, tamanio_a_alojar-desplazamiento-8);
	setear_memoria(marcos_a_cambiar, memoria_aux);
}

void cambiar_ubicacion_tripulante_PAGINACION(uint32_t id_proceso, uint32_t id_tripulante, uint32_t nueva_posx, uint32_t nueva_posy){
    pthread_mutex_lock(&sem_memoria);
	
    t_tabla_proceso* aux = buscar_proceso(id_proceso);
	uint32_t tripulante_logico = indice_de_tripulante(aux->lista_de_tids, id_tripulante);
	uint32_t direccion_logica = 8 + 5 + (21*tripulante_logico);
	t_list* paginas_a_cambiar = list_create();

	 
	if(direccion_logica/TAMANIO_PAGINAS == (direccion_logica+8)/TAMANIO_PAGINAS){
		list_add(paginas_a_cambiar, direccion_logica/TAMANIO_PAGINAS);
	} else {
		for (uint32_t i = direccion_logica/TAMANIO_PAGINAS; i<=(direccion_logica+8)/TAMANIO_PAGINAS; i++){
			list_add(paginas_a_cambiar, i);
		}
	}

	uint32_t direccion_logica_inicio = list_get(paginas_a_cambiar, 0);
	direccion_logica_inicio = direccion_logica_inicio * TAMANIO_PAGINAS;
	t_list* marcos_en_virtual = lista_marcos_en_virtual(paginas_a_cambiar, id_proceso);
	if(list_size(marcos_en_virtual)!=0){
		alojar(marcos_en_virtual);
	}
	t_list* marcos_a_cambiar;
	uint32_t mapeo_de_marco(uint32_t elem){
		return list_get(aux->lista_de_marcos, elem);	
	}
	marcos_a_cambiar = list_map(paginas_a_cambiar, mapeo_de_marco);
	setear_nueva_posicion(marcos_a_cambiar, nueva_posx, nueva_posy, direccion_logica - direccion_logica_inicio);
	list_destroy(marcos_en_virtual);
	list_destroy(marcos_a_cambiar);
	list_destroy(paginas_a_cambiar);
    pthread_mutex_unlock(&sem_memoria);
}

t_list* obtener_marcos_de_paginas(t_list* lista_de_marcos, uint32_t pagina_inicio, uint32_t pagina_fin){
	t_list* aux = list_create();
	for (uint32_t i=pagina_inicio; i<=pagina_fin; i++){
		list_add(aux, list_get(lista_de_marcos, i));
	}
	return aux;
}

void liberar_marcos(t_list* lista_marcos_borrado){
	void liberar_marco(uint32_t marco){
		if(marco>OFFSET){
			ESTADO_MARCOS_VIRTUALES[marco-OFFSET] = 0;
		} else {
			ESTADO_MARCOS[marco]=0;
		}
	}
	
	list_iterate(lista_marcos_borrado, liberar_marco);
}

void liberar_tabla(t_list* marcos, t_list* presencia, uint32_t pagina_inicio, uint32_t pagina_fin){
	for (uint32_t i=pagina_inicio; i<=pagina_fin; i++){
		list_remove(marcos, pagina_inicio);
		list_remove(presencia, pagina_inicio);
	}
}


uint32_t modificar_direccion_tareas(t_list* marcos_a_modificar, uint32_t dezplazamiento) {
	//devuelve direccion logica anterior de tareas
	uint32_t cantidad_de_marcos = list_size(marcos_a_modificar);
	void* memoria_auxi;
	memoria_auxi = (void*) malloc (cantidad_de_marcos*TAMANIO_PAGINAS);
	for(uint32_t i=0; i<cantidad_de_marcos; i++){
		uint32_t aux = list_get(marcos_a_modificar, i);
		memcpy(memoria_auxi+(i*TAMANIO_PAGINAS), MEMORIA + (aux*TAMANIO_PAGINAS), TAMANIO_PAGINAS);
	}
	uint32_t direccion_logica_tareas;
	memcpy(&direccion_logica_tareas, memoria_auxi + dezplazamiento, 4);
	direccion_logica_tareas = direccion_logica_tareas - 21;
	memcpy(memoria_auxi + dezplazamiento, &direccion_logica_tareas, 4);
	setear_memoria(marcos_a_modificar, memoria_auxi);
	return (direccion_logica_tareas + 21);
}

void necesito_en_ppal(uint32_t direccion_logica_inicio, uint32_t direccion_logica_fin, uint32_t id_proceso){
	t_tabla_proceso* aux = buscar_proceso(id_proceso);
	uint32_t pagina_inicio_check = direccion_logica_inicio/TAMANIO_PAGINAS;
	uint32_t pagina_fin_check = redondear_para_arriba (direccion_logica_fin, TAMANIO_PAGINAS);
	t_list* marcos_a_alojar = list_create();
	for(uint32_t i=pagina_inicio_check; i< pagina_fin_check; i++){
		uint32_t marco_aux = list_get(aux->lista_de_marcos, i);
		if(marco_aux>=OFFSET){
			list_add(marcos_a_alojar, marco_aux);
		}
	}
	alojar(marcos_a_alojar);
	list_destroy(marcos_a_alojar);
}

void reemplazar_marco_de_tabla_por_indice(uint32_t id_proceso, uint32_t nuevo_marco, uint32_t numero_pagina, uint32_t presencia) {
	t_tabla_proceso* aux2;
	aux2 = buscar_proceso(id_proceso);
	list_replace(aux2->lista_de_marcos, numero_pagina, nuevo_marco);
	switch (presencia){
	case 0:
		list_replace(aux2->lista_de_presencia, numero_pagina, 0);
		break;

	case 1:
		list_replace(aux2->lista_de_presencia, numero_pagina, 1);
		break;
	}
}

uint32_t obtener_indice_de_marco(uint32_t marco, uint32_t id_proceso){
	t_tabla_proceso* aux = buscar_proceso(id_proceso);
	uint32_t aux2 = 0;
	uint32_t indice;

	void buscarIndicex(uint32_t elem){
		if(elem == marco){
			indice = aux2;
		}
		aux2++;
	}

	list_iterate(aux->lista_de_marcos, buscarIndicex);
	return indice;
}

void alojar(t_list* lista_marcos_en_virtual){
	void* memoriaaux;
	uint32_t aux = 0;
	uint32_t aux2 = 0;
	uint32_t tamanio_memoria_aux = list_size(lista_marcos_en_virtual);
	t_list* paginas_a_corregir=list_create();
	t_list* procesos_a_corregir=list_create();
	void cargar_listas(marco_virtual){
		uint32_t proceso = obtener_proceso_de_marco(marco_virtual);
		list_add(procesos_a_corregir, proceso);
		list_add(paginas_a_corregir, obtener_indice_de_marco(marco_virtual, proceso));
	}
	list_iterate(lista_marcos_en_virtual, cargar_listas);
	tamanio_memoria_aux = tamanio_memoria_aux * TAMANIO_PAGINAS;
	memoriaaux = (void*) malloc (tamanio_memoria_aux);
	void backupear_liberar(uint32_t num){
		memcpy(memoriaaux + (aux*TAMANIO_PAGINAS), MEMORIA_VIRTUAL + ((num - OFFSET)*TAMANIO_PAGINAS), TAMANIO_PAGINAS);
		ESTADO_MARCOS_VIRTUALES[num-OFFSET] = 0;
		aux ++;	
	}
	list_iterate(lista_marcos_en_virtual, backupear_liberar);
	desalojar(list_size(lista_marcos_en_virtual));
	t_list* marcos_a_reservar = obtener_marcos_a_reservar(list_size(lista_marcos_en_virtual));
	setear_marcos_usados(marcos_a_reservar);
	setear_memoria(marcos_a_reservar, memoriaaux);
	void corregir_tabla_paginas(uint32_t marco){
		reemplazar_marco_de_tabla_por_indice(list_get(procesos_a_corregir, aux2), list_get(marcos_a_reservar, aux2), list_get(paginas_a_corregir, aux2), 1);
		aux2++;
	}
	list_iterate(lista_marcos_en_virtual, corregir_tabla_paginas);
	list_destroy(paginas_a_corregir);
	list_destroy(procesos_a_corregir);
	list_destroy(marcos_a_reservar);
}

t_list* obtener_marcos_segun_direccion_logica(uint32_t direccion_logica_inicio, uint32_t direccion_logica_fin, t_list* lista_de_marcos_completa){
	t_list* aux = list_create();
	uint32_t aux2 = 0;
	uint32_t pagina_inicio = direccion_logica_inicio / TAMANIO_PAGINAS;
	uint32_t pagina_fin = redondear_para_arriba(direccion_logica_fin, TAMANIO_PAGINAS);
	void filtrar_marcos(marco){
		if(aux2>=pagina_inicio && aux2<pagina_fin){
			list_add(aux, marco);
		}
		aux2++;
	}
	list_iterate(lista_de_marcos_completa, filtrar_marcos);
	return aux;
}

uint32_t obtener_indice_del_proceso(uint32_t pid){
	uint32_t aux = 0;
	uint32_t indice = 0;
	void buscar_id_proceso(t_tabla_proceso* tabla_proceso){
		if(tabla_proceso->pid==pid){
			indice = aux;
		}
		aux++;
	}
	list_iterate(TABLA_DE_PAGINAS, buscar_id_proceso);
	return indice;
}

uint32_t tamanio_lista_tareas(uint32_t id_proceso, uint32_t direccion_logica_tareas){
	t_tabla_proceso* aux = buscar_proceso(id_proceso);
	uint32_t ultima_direccion_logica = list_size(aux->lista_de_marcos) * TAMANIO_PAGINAS;

	t_list* marcos_de_lista_de_tareas;
	necesito_en_ppal(direccion_logica_tareas, ultima_direccion_logica, id_proceso);
	marcos_de_lista_de_tareas = obtener_marcos_segun_direccion_logica(direccion_logica_tareas, ultima_direccion_logica, aux->lista_de_marcos);
	
	uint32_t offset_tareas;
	offset_tareas = list_get(marcos_de_lista_de_tareas, 0);
	offset_tareas = offset_tareas * TAMANIO_PAGINAS;
	offset_tareas = offset_tareas + (direccion_logica_tareas%TAMANIO_PAGINAS);
	void* lista_tareas;
	lista_tareas = (void*) malloc (ultima_direccion_logica-direccion_logica_tareas);
	memcpy(lista_tareas, MEMORIA + offset_tareas, TAMANIO_PAGINAS - (direccion_logica_tareas%TAMANIO_PAGINAS));
	for(uint32_t i = 1; i < list_size(marcos_de_lista_de_tareas); i++) {
		uint32_t aux = list_get(marcos_de_lista_de_tareas, i);
		aux = aux * TAMANIO_PAGINAS;
		memcpy(lista_tareas +  TAMANIO_PAGINAS - (direccion_logica_tareas%TAMANIO_PAGINAS) + ((i-1) * TAMANIO_PAGINAS), MEMORIA + aux, TAMANIO_PAGINAS);
	}
	uint32_t tam_lista_tareas = strlen(lista_tareas)+1;
	free(lista_tareas);	
	list_destroy(marcos_de_lista_de_tareas);
	return tam_lista_tareas;
}

t_list* compactar_tripulante(uint32_t id_proceso, uint32_t direccion_logica_inicio, uint32_t ultima_direccion_logica, uint32_t lista_tareas){
	t_tabla_proceso* aux2 = buscar_proceso(id_proceso);
	t_list* lista_de_marcos_aux;
	uint32_t aux = 0;
	necesito_en_ppal(direccion_logica_inicio, ultima_direccion_logica, aux2->pid);
	lista_de_marcos_aux = obtener_marcos_segun_direccion_logica(direccion_logica_inicio, ultima_direccion_logica, aux2->lista_de_marcos);
	uint32_t tamanio_a_reservar = list_size(lista_de_marcos_aux)*TAMANIO_PAGINAS;
	void* memoria_aux;
	memoria_aux = (void*) malloc (tamanio_a_reservar);
	void copiar_a_memoria(uint32_t numero_marco){
		memcpy(memoria_aux + aux*TAMANIO_PAGINAS, MEMORIA + numero_marco*TAMANIO_PAGINAS, TAMANIO_PAGINAS);
		aux++;
	}
	list_iterate(lista_de_marcos_aux, copiar_a_memoria);
	
	uint32_t direccion_logica_inicio_real = (direccion_logica_inicio/TAMANIO_PAGINAS)*TAMANIO_PAGINAS;
	uint32_t desplazamiento = direccion_logica_inicio-direccion_logica_inicio_real;
	uint32_t cantidad_a_copiar = list_size(aux2->lista_de_tids);
	cantidad_a_copiar = (cantidad_a_copiar - 1) * 21 + lista_tareas;  
	
	memcpy(memoria_aux+desplazamiento, memoria_aux+desplazamiento+21, cantidad_a_copiar);

	uint32_t paginas_antes = redondear_para_arriba((8 + list_size(aux2->lista_de_tids) * 21 + lista_tareas), TAMANIO_PAGINAS);
	uint32_t paginas_ahora = redondear_para_arriba((8 + list_size(aux2->lista_de_tids) * 21 - 21 + lista_tareas), TAMANIO_PAGINAS); 
	uint32_t paginas_a_agarrar = list_size(lista_de_marcos_aux) - (paginas_antes-paginas_ahora);

	t_list* marcos_a_memoria;
	marcos_a_memoria = list_take(lista_de_marcos_aux, paginas_a_agarrar);

	setear_memoria(marcos_a_memoria, memoria_aux);

	list_destroy(marcos_a_memoria);

	t_list* marcos_a_borrar;
	marcos_a_borrar = list_create();
	for(uint32_t i = paginas_a_agarrar; i<list_size(lista_de_marcos_aux); i++){
		uint32_t auxiliar = list_get(lista_de_marcos_aux, i);
		list_add(marcos_a_borrar, auxiliar);
	}

	list_destroy(lista_de_marcos_aux);

	return marcos_a_borrar;
}


void expulsar_tripulante_PAGINACION(uint32_t id_proceso, uint32_t id_tripulante){
    pthread_mutex_lock(&sem_memoria);
	
	t_tabla_proceso* aux = buscar_proceso(id_proceso);

	if(list_size(aux->lista_de_tids)==1){
		liberar_marcos(aux->lista_de_marcos);
		list_destroy(aux->lista_de_marcos);
		list_destroy(aux->lista_de_presencia);
		list_destroy(aux->lista_de_tids);
		list_remove_and_destroy_element(TABLA_DE_PAGINAS, obtener_indice_del_proceso(id_proceso),free);
	}else{
		
		
		uint32_t tripulante_logico = indice_de_tripulante(aux->lista_de_tids, id_tripulante);
		uint32_t direccion_logica_inicio = 8 + (21*tripulante_logico);
		uint32_t direccion_logica_fin = 8 + (21*tripulante_logico) + 21;
		uint32_t direccion_logica_tareas;
		uint32_t ultima_direccion_logica = list_size(aux->lista_de_marcos) * TAMANIO_PAGINAS;


		t_list* lista_de_marcos_de_tareas;
		necesito_en_ppal(4, 8, id_proceso);
		lista_de_marcos_de_tareas = obtener_marcos_segun_direccion_logica(4, 8, aux->lista_de_marcos);
		direccion_logica_tareas = modificar_direccion_tareas(lista_de_marcos_de_tareas, (4%TAMANIO_PAGINAS));


		uint32_t lista_tareas = tamanio_lista_tareas(id_proceso, direccion_logica_tareas);
		t_list* lista_marcos_borrado = compactar_tripulante(id_proceso, direccion_logica_inicio, ultima_direccion_logica, lista_tareas);
		uint32_t pagina_inicio_borrado = obtener_indice_de_marco(list_get(lista_marcos_borrado, 0), id_proceso);
		uint32_t pagina_fin_borrado = obtener_indice_de_marco (list_get(lista_marcos_borrado, list_size(lista_marcos_borrado) - 1), id_proceso);

		if(list_size(lista_marcos_borrado) != 0){
			liberar_marcos(lista_marcos_borrado);
			liberar_tabla(aux->lista_de_marcos, aux->lista_de_presencia, pagina_inicio_borrado, pagina_fin_borrado);
		}
		list_remove(aux->lista_de_tids, tripulante_logico);
		list_destroy(lista_marcos_borrado);
		list_destroy(lista_de_marcos_de_tareas);
	}
    pthread_mutex_unlock(&sem_memoria);
}

t_list* deconstruir_string(char* array){
	t_list* aux = list_create();
	uint32_t cantidad_de_palabras = 0;
	char** array_spliteado = string_split(array, "-");
	void contar_palabras(char* palabra){
		cantidad_de_palabras++;
	}
	string_iterate_lines(array_spliteado, contar_palabras);
	for(uint32_t i=0; i<cantidad_de_palabras; i++){
		list_add(aux, array_spliteado[i]);
	}

	return aux;
}

uint32_t obtener_direccion_logica_tareas(uint32_t id_proceso){
	t_tabla_proceso* aux = buscar_proceso(id_proceso);
	necesito_en_ppal(4, 8, id_proceso);
	t_list* marcos_a_modificar;
	marcos_a_modificar = obtener_marcos_segun_direccion_logica(4, 8, aux->lista_de_marcos);
	uint32_t cantidad_de_marcos = list_size(marcos_a_modificar);
	void* memoria_auxi;
	memoria_auxi = (void*) malloc (cantidad_de_marcos*TAMANIO_PAGINAS);
	for(uint32_t i=0; i<cantidad_de_marcos; i++){
		uint32_t aux = list_get(marcos_a_modificar, i);
		memcpy(memoria_auxi+(i*TAMANIO_PAGINAS), MEMORIA + (aux*TAMANIO_PAGINAS), TAMANIO_PAGINAS);
	}
	uint32_t direccion_logica_tareas;
	memcpy(&direccion_logica_tareas, memoria_auxi + 4%TAMANIO_PAGINAS, 4);
	
	list_destroy(marcos_a_modificar);
	free(memoria_auxi);
	return direccion_logica_tareas;
}

char* obtener_lista_de_tareas(uint32_t direccion_logica, uint32_t id_proceso){
	t_tabla_proceso* aux = buscar_proceso(id_proceso);
	uint32_t cantidad_marcos_proceso = list_size(aux->lista_de_marcos);
	uint32_t cantidad_de_bytes = cantidad_marcos_proceso * TAMANIO_PAGINAS;
	necesito_en_ppal(direccion_logica, cantidad_de_bytes, id_proceso);
	t_list* marcos_a_copiar;
	marcos_a_copiar = obtener_marcos_segun_direccion_logica(direccion_logica, cantidad_de_bytes, aux->lista_de_marcos);
	uint32_t cantidad_de_marcos = list_size(marcos_a_copiar);
	

	void* memoria_auxi;
	memoria_auxi = (void*) malloc (cantidad_de_marcos* TAMANIO_PAGINAS);

	for(uint32_t i=0; i<cantidad_de_marcos; i++){
		uint32_t aux = list_get(marcos_a_copiar, i);
		memcpy(memoria_auxi+(i*TAMANIO_PAGINAS), MEMORIA + (aux*TAMANIO_PAGINAS), TAMANIO_PAGINAS);
	}

	uint32_t tamanio_tareas = 0;
	uint32_t aux3 = 1;
	
	for(uint32_t i= direccion_logica%TAMANIO_PAGINAS; i < cantidad_de_marcos* TAMANIO_PAGINAS; i++){
		char p;
		memcpy(&p, memoria_auxi+i, 1);
		if(p==NULL && tamanio_tareas==0){
			
			tamanio_tareas = aux3;
		}
		aux3++;
	}

	char* tareas;
	tareas = (char*) malloc(sizeof(char)*tamanio_tareas);

	memcpy(tareas, memoria_auxi+direccion_logica%TAMANIO_PAGINAS, tamanio_tareas);
	
	free(memoria_auxi);
	list_destroy(marcos_a_copiar);

	return tareas;
}

uint32_t obtener_id_proxima_tarea(uint32_t id_proceso, uint32_t id_tripulante){
	t_tabla_proceso* aux = buscar_proceso(id_proceso);
	uint32_t tripulante_logico = indice_de_tripulante(aux->lista_de_tids, id_tripulante);
	uint32_t direccion_logica_inicio = 8 + 5 + 8 +(21*tripulante_logico);
	uint32_t direccion_logica_fin = 8 + 5 + 8 +(21*tripulante_logico) + 4;

	necesito_en_ppal(direccion_logica_inicio, direccion_logica_fin, id_proceso);
	t_list* marcos_a_copiar;
	marcos_a_copiar = obtener_marcos_segun_direccion_logica(direccion_logica_inicio, direccion_logica_fin, aux->lista_de_marcos);
	uint32_t cantidad_de_marcos = list_size(marcos_a_copiar);
	void* memoria_auxi;
	memoria_auxi = (void*) malloc (cantidad_de_marcos*TAMANIO_PAGINAS);
	for(uint32_t i=0; i<cantidad_de_marcos; i++){
		uint32_t aux = list_get(marcos_a_copiar, i);
		memcpy(memoria_auxi+(i*TAMANIO_PAGINAS), MEMORIA + (aux*TAMANIO_PAGINAS), TAMANIO_PAGINAS);
	}
	uint32_t direccion_proxima_tarea;
	memcpy(&direccion_proxima_tarea, memoria_auxi + direccion_logica_inicio%TAMANIO_PAGINAS, 4);

	direccion_proxima_tarea++;
	memcpy(memoria_auxi +  direccion_logica_inicio%TAMANIO_PAGINAS, &direccion_proxima_tarea, 4);
	setear_memoria(marcos_a_copiar, memoria_auxi);

	list_destroy(marcos_a_copiar);

	return (direccion_proxima_tarea - 1);
};

char* proxima_tarea_PAGINACION (uint32_t id_proceso, uint32_t id_tripulante){
    pthread_mutex_lock(&sem_memoria);
	
    uint32_t direccion_logica_tareas = obtener_direccion_logica_tareas(id_proceso);
	char* todas_las_tareas = obtener_lista_de_tareas(direccion_logica_tareas, id_proceso);
	t_list* lista_de_tareas = deconstruir_string(todas_las_tareas);
	uint32_t proxima_tarea = obtener_id_proxima_tarea(id_proceso, id_tripulante);
	uint32_t cantidad_tareas = list_size(lista_de_tareas);
	free(todas_las_tareas);
	if(proxima_tarea < cantidad_tareas) {
		char* aux = list_get(lista_de_tareas, proxima_tarea);
		void* memoria_auxiliar;
		memoria_auxiliar = (void*) malloc (strlen(aux)+1);
		memcpy(memoria_auxiliar, aux, strlen(aux)+1);
		list_destroy_and_destroy_elements(lista_de_tareas, free);
        pthread_mutex_unlock(&sem_memoria);
        return memoria_auxiliar;
	} else {
        pthread_mutex_unlock(&sem_memoria);
		char* nulo = malloc(strlen("NULL")+1);         
        strcpy(nulo,"NULL");         
        return nulo;
	}

}

void actualizar_estado_PAGINACION(uint32_t id_proceso, uint32_t id_tripulante, char nuevo_estado){
    pthread_mutex_lock(&sem_memoria);	
    t_tabla_proceso* aux = buscar_proceso(id_proceso);
	uint32_t tripulante_logico = indice_de_tripulante(aux->lista_de_tids, id_tripulante);
	uint32_t direccion_logica_inicio = 8 + 4 +(21*tripulante_logico);
	uint32_t direccion_logica_fin = direccion_logica_inicio + 1;

	necesito_en_ppal(direccion_logica_inicio, direccion_logica_fin, id_proceso);
	t_list* marcos_a_copiar;
	marcos_a_copiar = obtener_marcos_segun_direccion_logica(direccion_logica_inicio, direccion_logica_fin, aux->lista_de_marcos);
	uint32_t cantidad_de_marcos = list_size(marcos_a_copiar);
	void* memoria_auxi;
	memoria_auxi = (void*) malloc (cantidad_de_marcos*TAMANIO_PAGINAS);
	for(uint32_t i=0; i<cantidad_de_marcos; i++){
		uint32_t aux = list_get(marcos_a_copiar, i);
		memcpy(memoria_auxi+(i*TAMANIO_PAGINAS), MEMORIA + (aux*TAMANIO_PAGINAS), TAMANIO_PAGINAS);
	}
	char estado;
	memcpy(&estado, memoria_auxi + direccion_logica_inicio%TAMANIO_PAGINAS, 1);
	estado = nuevo_estado;

	memcpy(memoria_auxi +  direccion_logica_inicio%TAMANIO_PAGINAS, &estado, 1);
	setear_memoria(marcos_a_copiar, memoria_auxi);

	list_destroy(marcos_a_copiar);
    pthread_mutex_unlock(&sem_memoria);	
}

char* estado_marco(uint32_t marco){
	if(ESTADO_MARCOS[marco]==0){
		return "LIBRE";
	} else {
		return "OCUPADO";
	}
}

void hacer_dump_PAGINACION(){
	char *timestamp = (char *)malloc(sizeof(char) * 16);
	char *timestamp2 = (char *)malloc(sizeof(char) * 16);
	time_t ltime;
	ltime=time(NULL);
	struct tm *tm;
	tm=localtime(&ltime);

	sprintf(timestamp,"Dump: %02d/%02d/%04d %02d:%02d:%02d", tm->tm_mday, tm->tm_mon, 
    tm->tm_year+1900, tm->tm_hour, tm->tm_min, tm->tm_sec);

	sprintf(timestamp2,"%02d%02d%04d_%02d%02d%02d", tm->tm_mday, tm->tm_mon, 
    tm->tm_year+1900, tm->tm_hour, tm->tm_min, tm->tm_sec);

	t_list* lista_dump;
	lista_dump = list_create();

	uint32_t aux_pid = 0;
	uint32_t aux_pag = 0;
	

	void barrer_un_proceso(uint32_t marco){
		if(marco<OFFSET){
            char* procesoId = string_itoa(aux_pid);             
            char* paginaId = string_itoa(aux_pag);
			dump_memoria dto;
			dto.marco = marco;
			dto.estado = estado_marco(marco);
			dto.proceso =  procesoId;
			dto.pagina = paginaId; 

			void* aux;
			aux = malloc(sizeof(dump_memoria));

			memcpy(aux, &dto, sizeof(dump_memoria));

			list_add(lista_dump, aux);
		}
		aux_pag++;
	}

	void llenar_lista(t_tabla_proceso* item_tabla){
		aux_pid = item_tabla->pid;
		list_iterate(item_tabla->lista_de_marcos, barrer_un_proceso);
		aux_pag = 0; 
	}

	bool ordenar(dump_memoria* a, dump_memoria*b){
		return (a->marco < b->marco);
	}

	list_iterate(TABLA_DE_PAGINAS, llenar_lista);
    for(uint32_t i = 0; i<CANTIDAD_MARCOS; i++){         
        if(ESTADO_MARCOS[i]==0){            
            dump_memoria dto;             
            dto.marco = i;             
            dto.estado = "LIBRE  ";            
            dto.proceso =  "-";            
            dto.pagina = "-";               
            void* aux;             
            aux = malloc(sizeof(dump_memoria));             
            memcpy(aux, &dto, sizeof(dump_memoria));              
            list_add(lista_dump, aux);         
        }
    }
	list_sort(lista_dump, ordenar);	

	char filename[32];
	strcpy(filename, "dumps/");
	strcat(filename, "Dump_<");
	strcat(filename, timestamp2);
	strcat(filename, ">");

    FILE *fp = fopen(filename, "w");
    if (fp == NULL)
    {
        printf("Error opening the file %s", filename);
        return -1;
    }
	fprintf(fp, "%s\n", timestamp);

	void imprimir_en_archivo(dump_memoria* data){
		fprintf(fp, "Marco:%d Estado:%s Proceso:%s Pagina:%s \n",data->marco, data->estado, data->proceso, data->pagina);
	}

	list_iterate(lista_dump, imprimir_en_archivo);

    fclose(fp);

	list_destroy_and_destroy_elements(lista_dump, free);
}

void mostrar_array_marcos(){
	printf("\n*****ARRAY DE MARCOS*****\n");
	for(uint32_t i = 0; i<CANTIDAD_MARCOS; i++){
		if(i<10)
			printf("--%d ", i);
		if(i>=10 && i<100)
			printf("-%d ", i);	
		if(i>=100 && i<1000)
			printf("%d ", i);
	}
	printf("\n");

	for(uint32_t a = 0; a<CANTIDAD_MARCOS; a++){
		printf("--%d ", ESTADO_MARCOS[a]);
	}
	printf("\n*************************\n\n");
}

void mostrar_array_timestamp(){
	printf("\n*****ARRAY DE TIMESTAMP*****\n");
	for(uint32_t i = 0; i<CANTIDAD_MARCOS; i++){
		if(i<10)
			printf("--%d ", i);
		if(i>=10 && i<100)
			printf("-%d ", i);	
		if(i>=100 && i<1000)
			printf("%d ", i);
	}
	printf("\n");

	for(uint32_t a = 0; a<CANTIDAD_MARCOS; a++){
		printf("-%d ", TIMESTAMP_MARCOS[a]);
	}
	printf("\n*************************\n\n");
}

void mostrar_array_bit_uso(){
	printf("\n*****BIT DE USO*****\n");
	for(uint32_t i = 0; i<CANTIDAD_MARCOS; i++){
		if(i<10)
			printf("--%d ", i);
		if(i>=10 && i<100)
			printf("-%d ", i);	
		if(i>=100 && i<1000)
			printf("%d ", i);
	}
	printf("\n");

	for(uint32_t a = 0; a<CANTIDAD_MARCOS; a++){
		printf("-%d ", ARRAY_BIT_USO[a]);
	}
	printf("\n*************************\n\n");
}

void mostrar_array_marcos_virtuales(){
	printf("\n*****ARRAY DE MARCOS VIRTUALES*****\n");
	for(uint32_t i = 0; i<CANTIDAD_MARCOS_VIRTUALES; i++){
		if(i<10)
			printf("--%d ", i);
		if(i>=10 && i<100)
			printf("-%d ", i);	
		if(i>=100 && i<1000)
			printf("%d ", i);
	}
	printf("\n");

	for(uint32_t a = 0; a<CANTIDAD_MARCOS_VIRTUALES; a++){
		printf("--%d ", ESTADO_MARCOS_VIRTUALES[a]);
	}
	printf("\n*************************\n\n");
}

void mostrar_lista(t_list* lista){
	void printer(uint32_t num){
		printf("%d ", num);
	}
	printf("La lista es: ");
	list_iterate(lista, printer);
	printf("\n");
}

void mostrar_tabla_de_paginas(){
	
	printf("\n*****TABLA DE PAGINAS*****\n");
	void imprimir_un_valor(t_tabla_proceso* item_tlb){
		uint32_t i = 0;
		printf("PID: %d - ", item_tlb->pid);

		printf("TIDS: ", item_tlb->pid);
		void imprimir_marco2(uint32_t num){
			printf("%d ", num);
		}

		list_iterate(item_tlb->lista_de_tids, imprimir_marco2);
		
		printf("- ");
		void imprimir_marco(uint32_t num){
		printf("%d/", num);
		printf("%d ", list_get(item_tlb->lista_de_presencia, i));
		i++;
		}

		list_iterate(item_tlb->lista_de_marcos, imprimir_marco);
		printf("\n");
	}

	list_iterate(TABLA_DE_PAGINAS, imprimir_un_valor);
	printf("********************************\n\n");
}


/* ------------------------Fin PAGINACION--------------------------- */

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

    // printf("se imprimira la memoria\n\n");
    for (int i = 0; i < tamanio; i++)
    {   
        pthread_mutex_lock(&sem_memoria); 
        memcpy(MEMORIA + i,(void*)"",sizeof(char));
        pthread_mutex_unlock(&sem_memoria); 
        // memory[i] = (char) "";
    }
    // printf("\n\nse imprimio la memoria\n");
}

char* retornar_tareas(t_segmento* segmento_tareas)
{
    int tamanio_segmento = segmento_tareas->fin - segmento_tareas->inicio + 1;
    char* tareas_en_MEMORIA = (char*) malloc(tamanio_segmento+1);
    int desplazamiento = segmento_tareas->inicio;

    // mutex por bytes a utilizar - no recuerdo el nombre
    // pthread_mutex_lock(&sem_memoria); 
    memcpy(tareas_en_MEMORIA, MEMORIA+desplazamiento, tamanio_segmento);
    // pthread_mutex_unlock(&sem_memoria); 
    // mutex por bytes a utilizar - no recuerdo el nombre
    
    // log_info(logger,"tareas_en_memoria %s",tareas_en_MEMORIA);
    tareas_en_MEMORIA[tamanio_segmento] = '\0';
    // log_info(logger,"tareas_en_memoria %s",tareas_en_MEMORIA);


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
        char* nulo = malloc(strlen("NULL")+1);
        strcpy(nulo,"NULL");
        return nulo;
    } 

    char** tarea_subs = string_split(tareas_en_MEMORIA,"-");
    // log_info(logger,"la tarea a buscar -> %s",tarea_subs[nro_de_tarea]);
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
        
        if (patota_segmento_pcb->se_encuentra == 1)
        {
            uint32_t patota_pcb_pid = retornar_pid_del_pcb(patota_segmento_pcb);
            
            if (patota_pcb_pid == pid)
            {
                return segmentos_patota;
            }
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
void hacer_dump_SEGMENTACION()
{
        // Descomentar para testeo
    // log_info(logger,"\t     En RAM se encuentran %d patotas",list_size(lista_de_patotas));
    
    char *timestamp = (char *)malloc(sizeof(char) * 28);
	char *timestamp2 = (char *)malloc(sizeof(char) * 16);
	time_t ltime;
	ltime=time(NULL);
	struct tm *tm;
	tm=localtime(&ltime);

	sprintf(timestamp,"Dump: %02d/%02d/%04d %02d:%02d:%02d", tm->tm_mday, tm->tm_mon, 
    tm->tm_year+1900, tm->tm_hour, tm->tm_min, tm->tm_sec);

	sprintf(timestamp2,"%02d%02d%04d_%02d%02d%02d", tm->tm_mday, tm->tm_mon, 
    tm->tm_year+1900, tm->tm_hour, tm->tm_min, tm->tm_sec);


    char filename[32];
    strcpy(filename, "dumps/");
    strcat(filename, "Dump_<");
    strcat(filename, timestamp2);
    strcat(filename, ">");

    FILE *fp;
    fp = fopen(filename, "w");
    if (fp == NULL)
    {
    printf("Error opening the file %s", filename);
    return -1;
    }

    rewind(fp);
    // fprintf(fp, "\t\t\t%s\n", timestamp);

    t_list* patotas_presentes = list_filter(lista_de_patotas,condicion_patota_presente_en_memoria);

    list_iterate(patotas_presentes, (void*) iterator_patotas_presentes);

    void imprimir_en_archivo_SEGMENTACION(dump_memoria_segmentacion* data){
        fprintf(fp, "Proceso:%d\t\tSegmento:%d\t\tInicio:%d\t\tTam:%d \n",data->pid, data->indice, data->inicio, data->tamanio);
    };


    void imprimir_en_archivo_libre_SEGMENTACION(t_segmento* segmento){
        fprintf(fp, "\t\t\t\tSegmento :%c\t\tInicio:%d\t\tTam:%d \n",segmento->tipo, segmento->inicio, (segmento->fin - segmento->inicio)+1);
    };
    
    if(list_is_empty(list_dump_segmentacion))
    {
        fprintf(fp, "\t%s\n", timestamp);
        fprintf(fp, "%s", "  No hay ningun proceso en memoria\n");
        list_iterate(admin_segmentacion->segmentos_libres,imprimir_en_archivo_libre_SEGMENTACION);
    }else
    {
        fprintf(fp, "  \t\t\t%s\n", timestamp);
        list_iterate(list_dump_segmentacion, imprimir_en_archivo_SEGMENTACION);
        list_iterate(admin_segmentacion->segmentos_libres,imprimir_en_archivo_libre_SEGMENTACION);
    }
    
        

    list_clean_and_destroy_elements(list_dump_segmentacion,iterator_destroy);
    fclose(fp);

    list_destroy(patotas_presentes);
    free(timestamp);
    free(timestamp2);

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

char* unir_tareas(t_list* lista)
{
    // int tamanio = tamanio_de_tareas(lista);
    // t_list_iterator* list_iterator = list_iterator_create(lista);
    
    // char* tareas_completas_unidas = malloc(tamanio);
    // while(list_iterator_has_next(list_iterator))
    // {
    //     char* nueva_tarea = (char*) list_iterator_next(list_iterator);
    //     if (!list_iterator_has_next(list_iterator))
    //     {
    //         // nueva_tarea[strlen(nueva_tarea)-1] = "\0";
    //         strncat(tareas_completas_unidas,nueva_tarea,strlen(nueva_tarea)-1);
    //     }else
    //     {
    //         strcat(tareas_completas_unidas,nueva_tarea);
    //     }
        
    // }
    // list_iterator_destroy(list_iterator);
    
    // strcpy(tareas_completas_unidas,tareas_completas);




    t_list_iterator* list_iterator = list_iterator_create(lista);

    char tareas_unidas[600];
    strcpy(tareas_unidas,"");
    char* tareas_completas;
    while(list_iterator_has_next(list_iterator))
    {
        char* nueva_tarea = (char*) list_iterator_next(list_iterator);
        strcat(tareas_unidas,nueva_tarea);
    }
    list_iterator_destroy(list_iterator);

    strcat(tareas_unidas,"\0");
    char* tareas = (char*) malloc(strlen(tareas_unidas)+1);
    strcpy(tareas,tareas_unidas);
    
    tareas[strlen(tareas)-1] = '\0';
    // loggear_entero(strlen(tareas));
    // log_info(logger,"tareas-> %s",tareas);
    return tareas;
    // return tareas_completas_unidas;
};

int tamanio_de_tareas(t_list* lista)
{
    t_list_iterator* list_iterator = list_iterator_create(lista);
    int tamanio=0;
    int espacio_ocupado_por_las_tareas(char* tarea){
        
    }
    while(list_iterator_has_next(list_iterator))
    {
        char* nueva_tarea = (char*) list_iterator_next(list_iterator);
        tamanio += strlen(nueva_tarea);
    }
    list_iterator_destroy(list_iterator);
    return tamanio;
}

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
    // log_info(logger,"estado %c", est);
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
    uint32_t pid = retornar_pid_del_pcb(segmento_pcb);
    t_segmento* segmento_presente;
    t_list* segmentos_de_la_patota_presentes = list_filter(patota,condicion_segmento_presente_en_memoria);
    t_list_iterator* list_iterator_patota = list_iterator_create(segmentos_de_la_patota_presentes);
    
    while(list_iterator_has_next(list_iterator_patota))
    {
        segmento_presente = (t_segmento*) list_iterator_next(list_iterator_patota);
        int tamanio = segmento_presente->fin - segmento_presente->inicio +1;
        
        dump_memoria_segmentacion dto;
        dto.pid = pid;
        dto.indice = list_iterator_patota->index+1;
        dto.inicio =  segmento_presente->inicio;
        dto.tamanio = tamanio; 

        void* aux;
        aux = malloc(sizeof(dump_memoria_segmentacion));

        memcpy(aux, &dto, sizeof(dump_memoria_segmentacion));

        list_add(list_dump_segmentacion, aux);

    }
    free(list_iterator_patota);
    list_destroy(segmentos_de_la_patota_presentes);

}
void iterator_patotas_presentes_cambiado(t_list* patota)
{
    t_list* segmentos_de_la_patota_presentes = list_filter(patota,condicion_segmento_presente_en_memoria);
    list_iterate(segmentos_de_la_patota_presentes,iterator_segmento);
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
    // loggear_entero(segmento_tareas->inicio);
    // loggear_entero(segmento_tareas->fin);
    // segmento_tareas->inicio = 134;
    // segmento_tareas->fin = 179;
    char* works = retornar_tareas(segmento_tareas);
    log_info(logger,"Las taeritas son: %s",works);
    free(works);
    loggear_info_segmento(segmento_tareas,"segmento_tareas");
    loggear_linea();
}

bool orden_mayor_a_menor(int entero_A, int entero_B)
{
    return entero_A > entero_B;
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

bool orden_lista_admin_segmentacion_best_free_mayor_a_menor(t_segmento* segmento_libre_A, t_segmento* segmento_libre_B)
{
    int diferencia_A = segmento_libre_A->fin - segmento_libre_A->inicio + 1;
    int diferencia_B = segmento_libre_B->fin - segmento_libre_B->inicio + 1;
    return diferencia_A > diferencia_B;
}

bool orden_lista_pcbs(t_pcb* pcb_a, t_pcb* pcb_b)
{
    return pcb_a->pid < pcb_b->pid;
}

bool orden_lista_segmentos(t_segmento* seg_a, t_segmento* seg_b)
{
    return seg_a->inicio < seg_b->inicio;
}

bool orden_lista_mapa(t_mapa* mapa_a, t_mapa* mapa_b)
{
    return mapa_a->pid < mapa_b->pid;
}

bool orden_lista_segmentos_contraria(t_segmento* seg_a, t_segmento* seg_b)
{
    return seg_a->inicio > seg_b->inicio;
}

bool comparador_patotas(t_list* seg_patota_a, t_list* seg_patota_b)
{
    t_pcb* pcb_a, *pcb_b;
    pcb_a = list_get(seg_patota_a,0);
    pcb_b = list_get(seg_patota_b,0);
    return pcb_a->pid < pcb_b->pid;
}

bool condicion_segmento_afectado(t_segmento* seg)
{
    return seg->fin > ultimo_segmento_libre_compactable()->inicio;
}

bool condicion_segmento_presente_en_memoria(t_segmento* seg)
{
    return seg->se_encuentra == 1;
}

bool condicion_patota_presente_en_memoria(t_list* segmentos_patota)
{
    t_segmento* segmento_pcb = list_get(segmentos_patota,0);
    // segmento_pcb->se_encuentra;
    // free(segmento_pcb);
    return segmento_pcb->se_encuentra == 1;
}

void transformacion_segmento_afectado(t_segmento* seg)
{
    t_segmento* segmento_libre = ultimo_segmento_libre_compactable();
    int diferencia_libre = segmento_libre->fin - segmento_libre->inicio + 1;
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
    
    // printf("de %d a %d \n",seg->inicio, (seg->inicio) - diferencia_libre);
    // printf("cantidad ocupada %d \n",diferencia_ocupado);
    void* memoria_aux = malloc(SMOBJ_SIZE);
    memcpy(memoria_aux, MEMORIA ,SMOBJ_SIZE);
    memcpy(MEMORIA + (seg->inicio) - diferencia_libre, memoria_aux + seg->inicio, diferencia_ocupado);
    free(memoria_aux);
    seg->inicio -= diferencia_libre;
    seg->fin -= diferencia_libre;

    segmento_libre->inicio += diferencia_ocupado;
    segmento_libre->fin += diferencia_ocupado;
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
    // loggear_linea();
    
    char* esquema = config_get_string_value(config,"ESQUEMA_MEMORIA");
    

    if ( strcmp(esquema,"SEGMENTACION") == 0)
    {
        
        hacer_dump_SEGMENTACION();
    }else
    {
        hacer_dump_PAGINACION();
    }


    // loggear_linea();
}

void my_signal_compactar(int sig)
{
    // loggear_linea();
    // log_info(logger,"Se utilizo el signal %d correctamente", sig);
    // log_info(logger,"Se procedera a compactar, espere por favor");
    pthread_mutex_lock(&sem_memoria);
    compactar();
    pthread_mutex_unlock(&sem_memoria);
    // loggear_linea();
}



/* ------------------------------------------------------------------- */
/* ---------------------------Envios---------------------------------- */
/* ------------------------------------------------------------------- */

void enviar_proxima_tarea(char* tarea_solicitada, int cliente_fd)
{
		// int cod_operacion = FINALIZACION; 
		int cod_operacion = ENVIAR_PROXIMA_TAREA; 
	t_paquete* paquete = crear_paquete(cod_operacion);
	
	// log_info(logger,tarea_solicitada);
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

char* trimwhitespace(char *str)
{
  char *end;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator character
  end[1] = '\0';

  return str;
}