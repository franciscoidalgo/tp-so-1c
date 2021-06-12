#include "miramhq.h"

t_log* logger;
t_config* config;
administrador_de_segmentacion* admin_segmentacion;


/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */

/* ---------------------------Iteradores------------------------------ */

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

void iterator_destroy_tarea(t_tarea* tarea)
{
    free(tarea->accion);
    free(tarea);
};

void iterator_lines_free(char* string)
{
    free(string);
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

void iterator_segmento_libre(t_segmento_libre* segmento_libre)
{
    log_info(logger,"Inicio: %d", segmento_libre->inicio);
    log_info(logger,"Fin: %d", segmento_libre->fin);
};

bool orden_lista_admin_segmentacion(t_segmento_libre* segmento_libre_A, t_segmento_libre* segmento_libreB)
{
    return segmento_libre_A->inicio < segmento_libreB->inicio;
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

/* -------------------------Fin Iteradores---------------------------- */

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

    printf("Cantidad de bytes libres: %d\n",admin_segmentacion->bytes_libres);
    list_iterate(admin_segmentacion->segmentos_libres, (void*) iterator_segmento_libre);

    // testear_asignar_y_liberar_segmentacion();


    liberar_espacio_de_memoria();
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
    // int i;
	// int hilos = 3;
    // pthread_t tid[hilos][2];



    // Iniciar Servidor
    int server_fd = iniciar_servidor(logger,config);


    // Continuara
    int cod_op = MENSAJE;
    t_list* lista = list_create();
    while(cod_op != FINALIZACION)
    {
        // Bloqueamos al servidor hasta que reciba algun connect
        int cliente_fd = esperar_cliente(server_fd,logger);

        // Crear el hilo aca con el cliente que acaba de llegar
        cod_op = recibir_operacion(cliente_fd);



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

            int cant_tripulantes = recibir_catidad_de_tripulantes(lista);
            log_info(logger," La cant de tripulantes son %d\n",cant_tripulantes);
            t_list* tareas;
            // crear_patota(5,lista);
            tareas = crear_tareas(lista);
			printf("\tMe llegaron los siguientes valores:\n");
			list_iterate(tareas, (void*) iterator_tarea);

            list_clean_and_destroy_elements(lista, (void*) iterator_destroy);   
			// list_destroy(lista);
            list_clean_and_destroy_elements(tareas, (void*) iterator_destroy_tarea);
			list_destroy(tareas);
            // liberar_espacio_de_memoria(); // BORRAR ESTOOO!
            break;
		case INICIAR_TRIPULANTE:
            log_info(logger,"Un tripulante se ha iniciado");
			recibir_paquete(cliente_fd,lista);
            // crear_patota(5,lista);
			printf("\tMe llegaron los siguientes valores:\n");
			// list_iterate(lista, (void*) iterator);
            list_clean_and_destroy_elements(lista, (void*) iterator_destroy);   
			break;
		case FINALIZACION:
			printf("\tSe acabo todo, nos vemos!\n");
			// int size;
			// recibir_mensaje(cliente_fd,logger,&size);

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

    }
    // list_clean_and_destroy_elements(lista, (void*) iterator_destroy);
    // list_iterate(lista, (void*) iterator_destroy);
    list_destroy(lista);
    /* -----------------------Fin Tercer Paso-------------------------------- */

    
    // terminamos el proceso, eliminamos todo
    free(MEMORIA); 
    terminar_miramhq(logger,config);

}






/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*FUNCIONES*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */




/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */

/* ----------------------Administrar Memoria-------------------------- */

void reservar_espacio_de_memoria()
{

    MEMORIA = (void*) malloc (config_get_int_value(config,"TAMANIO_MEMORIA"));

};

void liberar_espacio_de_memoria()
{
    list_clean_and_destroy_elements(admin_segmentacion->segmentos_libres, (void*) iterator_destroy);   
    list_destroy(admin_segmentacion->segmentos_libres);
    free(admin_segmentacion);

    free(MEMORIA);
};

/* --------------------Fin Administrar Memoria------------------------ */

/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */

/* --------------------------Segmentacion----------------------------- */

void iniciar_segmentacion()
{
    admin_segmentacion = malloc(sizeof(administrador_de_segmentacion));
    t_list* segmentos_libre = list_create();
    t_segmento_libre* segmento = malloc(sizeof(t_segmento_libre));

    segmento->inicio = (uint32_t) 0;
    segmento->fin = (uint32_t) config_get_int_value(config,"TAMANIO_MEMORIA");

    list_add(segmentos_libre,segmento);
    admin_segmentacion->bytes_libres = (uint32_t) config_get_int_value(config,"TAMANIO_MEMORIA");
    admin_segmentacion->segmentos_libres = segmentos_libre;
};


void asignar_segmento(uint32_t bytes_ocupados)
{
    t_segmento_libre* segmento = buscar_segmento_libre(bytes_ocupados);


    free(segmento);
};

t_segmento_libre* buscar_segmento_libre(uint32_t bytes_ocupados)
{
    t_list_iterator* list_iterator = list_iterator_create(admin_segmentacion->segmentos_libres);
    bool encontrado = false;

    t_segmento_libre* segmento = malloc(sizeof(t_segmento_libre));

    while(!encontrado && list_iterator_has_next(list_iterator))
    {
        segmento = (t_segmento_libre*) list_iterator_next(list_iterator);
        int diferencia = (segmento->fin - segmento->inicio);

        if (bytes_ocupados <= diferencia)
        {
            if (bytes_ocupados < diferencia)
            {
                t_segmento_libre* segmento_nuevo = malloc(sizeof(t_segmento_libre));
                segmento_nuevo->inicio = segmento->inicio+bytes_ocupados;
                segmento_nuevo->fin = segmento->fin;
                list_replace(admin_segmentacion->segmentos_libres,list_iterator->index,segmento_nuevo);
                // list_replace_and_destroy_element(admin_segmentacion->segmentos_libres,list_iterator->index,segmento_nuevo,iterator_destroy);
            }else
            {
                list_remove_and_destroy_element(admin_segmentacion->segmentos_libres,list_iterator->index,iterator_destroy);
            }
            segmento->fin = segmento->inicio + bytes_ocupados; 
            encontrado = true;
        }
        
        // free(segmento);
    }

    if (encontrado)
    {
        return segmento;
    }
    else{
        log_info(logger,"No hay espacio para colocar esta cantidad de bytes, te recomendaria compactar");
        return NULL;
    }
}


void liberar_segmento(t_segmento_libre* segmento)
{
    list_add(admin_segmentacion->segmentos_libres, segmento);
    list_sort(admin_segmentacion->segmentos_libres,orden_lista_admin_segmentacion);
}


/* ------------------------Fin Segmentacion--------------------------- */

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
    }

    list_iterator_destroy(list_iterator);

    return tareas;
};


/* ---------Fin Creaciones de Estructuras administrativas------------- */

/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */


/* --------------------------Recibir---------------------------------- */

uint32_t recibir_catidad_de_tripulantes(t_list* lista)
{
    // t_pcb* t_pcb = malloc(sizeof(t_pcb));
    char* pid = list_remove(lista,0); 
    uint32_t cant_de_tripulantes = (uint32_t) atoi(pid);
    // log_info(logger,(uint32_t) t_pcb->pid);
    free(pid);
    return cant_de_tripulantes;
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
    t_segmento_libre* segmento48,*segmento20,*segmento32;
    segmento48 = buscar_segmento_libre(48);
    list_iterate(admin_segmentacion->segmentos_libres, (void*) iterator_segmento_libre);
    log_info(logger,"El inicio del segmento 48 es %d y el fin %d\n", segmento48->inicio,segmento48->fin);
    // printf("El inicio del segmento 48 es %d y el fin %d\n",segmento48->inicio,segmento48->fin);

 
    segmento20 = buscar_segmento_libre(20);
    list_iterate(admin_segmentacion->segmentos_libres, (void*) iterator_segmento_libre);
    log_info(logger,"El inicio del segmento 20 es %d y el fin %d\n", segmento20->inicio,segmento20->fin);
    // printf("El inicio del segmento 20 es %d y el fin %d\n",segmento20->inicio,segmento20->fin);

    segmento32 = buscar_segmento_libre(32);
    list_iterate(admin_segmentacion->segmentos_libres, (void*) iterator_segmento_libre);
    log_info(logger,"El inicio del segmento 32 es %d y el fin %d\n", segmento32->inicio,segmento32->fin);
    // printf("El inicio del segmento 32 es %d y el fin %d\n",segmento32->inicio,segmento32->fin);

    liberar_segmento(segmento20);
    list_iterate(admin_segmentacion->segmentos_libres, (void*) iterator_segmento_libre);
    // printf("El inicio nuevo es %d y el fin %d\n",segmento20->inicio,segmento20->fin);

    free(segmento48);
    free(segmento20);
    free(segmento32);
}


/* -------------------------Fin Testeos------------------------------- */

/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */
/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */

void atender_cliente(int cliente_fd)
{
    // ACa iria un metodo de cierre
    // while()
    // {
    int cod_op = recibir_operacion(cliente_fd);

    switch (cod_op)
    {
    case MENSAJE: ;
        int size;
        recibir_mensaje(cliente_fd,logger,&size);
        break;
    
    case PAQUETE:
        // int size;
        // recibir_paquete(cliente_fd,logger,&size);
        break;
    
    default:
        log_info(logger,"No le llego bien el Cod de Operacion");
        break;
    }

    // }
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