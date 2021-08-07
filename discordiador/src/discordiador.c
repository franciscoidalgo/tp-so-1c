#include "discordiador.h"
#include "planificadorFIFO.h"
#include "planificadorRR.h"

int main()
{

	inicializar_variables();
	/*
enviar un mensaje a IMONGOSTORE para mantener una conexion activa (clavarme en un recv) y 
luego poder recibir la señal de sabotaje por ese "tunel" establecido
 */
	pthread_t hiloEscuchaSabotaje;
	pthread_create(&hiloEscuchaSabotaje, NULL, (void *)atender_sabotaje, NULL);
	pthread_detach(hiloEscuchaSabotaje);

	pthread_t hilo_recepcionista;
	while (1)
	{
		char *retorno_consola = readline(">");
		pthread_create(&hilo_recepcionista, NULL, (void *)atender_accion_de_consola, retorno_consola);
		pthread_detach(hilo_recepcionista);
	}

	log_destroy(logger);
	//terminar_variables_globales(conexion);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////IMPLEMENTACION DE FUNCIONES//////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void iterator(t_tripulante *t)
{
	log_info(logger, "TID:%d POSX:%d POSY:%d PCB:%d ESTADO:%c", t->tid, t->posicion_x, t->posicion_y, t->puntero_pcb, t->estado);
}

void terminar_variables_globales(int socket)
{
	log_destroy(logger);
	liberar_conexion(socket);
	//config_destroy(config);
}

bool es_tripu_de_patota(int patota, int id, t_tripulante *tripulante)
{
	return (tripulante->tid == id && tripulante->puntero_pcb == patota);
}

//inner_function
void mover_tripulante_entre_listas_si_existe(int lista_origen, int list_destino, int patota, int id_tripu)
{
	bool _el_tripulante_que_limpio(void *elemento)
	{
		return es_tripu_de_patota(patota, id_tripu, elemento);
	};

	if (list_any_satisfy((t_list*)obtener_lista(lista_origen), _el_tripulante_que_limpio))
	{
		if( lista_origen == _EXEC_) sem_post(&sem_exe);
		t_tripulante* tripu = (t_tripulante*) list_remove_by_condition((t_list*)obtener_lista(lista_origen), _el_tripulante_que_limpio);
		add_queue(list_destino, tripu);
	}
}

t_list *obtener_lista(int lista)
{
	switch (lista)
	{
	case _NEW_:
		return NEW;
		break;
	case _READY_:
		return READY;
		break;
	case _EXEC_:
		return EXEC;
		break;
	case _BLOCKED_:
		return BLOCKED;
		break;
	case _BLOCKED_EMERGENCY_:
		return BLOCKED_EMERGENCY;
		break;
	default:
		break;
	}

	return 0;
}

void inicializar_variables()
{
	ID_PATOTA = 0;
	logger = iniciar_logger("discordiador");
	log_info(logger, "Soy el discordiador! %s", mi_funcion_compartida());
	config = (void *)leer_config("discordiador");
	READY = list_create();
	BLOCKED_EMERGENCY = list_create();
	BLOCKED = list_create();
	EXIT = list_create();
	EXEC = list_create();
	NEW = list_create();
	EXISTE_SABOTAJE = false;
	IP_MI_RAM_HQ = config_get_string_value((t_config *)config, "IP_MI_RAM_HQ");
	PUERTO_MI_RAM_HQ = config_get_string_value((t_config *)config, "PUERTO_MI_RAM_HQ");
	IP_I_MONGO_STORE = config_get_string_value((t_config *)config, "IP_I_MONGO_STORE");
	PUERTO_I_MONGO_STORE = config_get_string_value((t_config *)config, "PUERTO_I_MONGO_STORE");
	GRADO_MULTITAREA = config_get_int_value((t_config *)config, "GRADO_MULTITAREA");
	ALGORITMO = (int)obtener_algoritmo(config_get_string_value((t_config *)config, "ALGORITMO"));
	QUANTUM = config_get_int_value((t_config *)config, "QUANTUM");
	DURACION_SABOTAJE = config_get_int_value((t_config *)config, "DURACION_SABOTAJE");
	RETARDO_CICLO_CPU = config_get_int_value((t_config *)config, "RETARDO_CICLO_CPU");
	pthread_mutex_init(&mutex_sabotaje, NULL);
	pthread_mutex_init(&mutex_entrada_salida, NULL);
	// pthread_mutex_lock(&mutex_entrada_salida);
	sem_init(&sem_IO, 0, 1);
	sem_init(&sem_IO_queue, 0, 0);
	sem_init(&sem_exe_notificacion, 0, 0);
	sem_init(&sem_exe, 0, GRADO_MULTITAREA);
	pthread_cond_init(&condicion_pausear_planificacion, NULL);
	pthread_cond_init(&semaforo_sabotaje, NULL);
	pthread_mutex_init(&mutex_sabotaje, NULL);
	//diccionario de acciones de consola
	dic_datos_consola = dictionary_create();
	dictionary_put(dic_datos_consola, "INICIAR_PATOTA", (void *)INICIAR_PATOTA_);
	dictionary_put(dic_datos_consola, "INICIAR_PLANIFICACION", (void *)INICIAR_PLANIFICACION);
	dictionary_put(dic_datos_consola, "PAUSAR_PLANIFICACION", (void *)PAUSAR_PLANIFICACION);
	dictionary_put(dic_datos_consola, "EXPULSAR_TRIPULANTE", (void *)EXPULSAR_TRIPULANTE_);
	dictionary_put(dic_datos_consola, "LISTAR_TRIPULANTE", (void *)LISTAR_TRIPULANTE);
	dictionary_put(dic_datos_consola, "OBTENER_BITACORA", (void *)OBTENER_BITACORA);
}

int get_diccionario_accion(char *accion)
{
	if (dictionary_has_key(dic_datos_consola, accion))
	{
		return (int)dictionary_get(dic_datos_consola, (char *)accion);
	}
	else
	{
		return -1;
	}
}

void enviar_mensaje_and_codigo_op(char *mensaje, int cod_op, int socket_cliente)
{
	t_paquete *paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = cod_op;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2 * sizeof(int);

	//void* a_enviar = serializar_paquete(paquete, bytes);
	void *INICIO = malloc(bytes);
	int desplazamiento = 0;

	memcpy(INICIO + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(INICIO + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(INICIO + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento += paquete->buffer->size;

	send(socket_cliente, INICIO, bytes, 0); //envia y espera la respuesta

	free(INICIO);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

t_tripulante *crear_tripulante(uint32_t patota, uint32_t posx, uint32_t posy, uint32_t tid)
{
	t_tripulante *t = malloc(sizeof(t_tripulante));

	t->posicion_x = posx;
	t->posicion_y = posy;
	//t->proxima_instruccion = 0; //tarea,luego la busca en RAM
	t->tid = tid;
	t->puntero_pcb = patota;
	t->estado = 'N';
	// t->socket_MIRAM = crear_conexion(IP_MI_RAM_HQ, PUERTO_MI_RAM_HQ);
	// t->socket_IMONGOSTORE = crear_conexion(IP_MI_RAM_HQ, PUERTO_MI_RAM_HQ);

	return t;
}

void enviar_tareas_a_RAM(int conexion, char **linea_consola)
{

	char *path = string_new();
	string_append(&path, "tareas/");
	string_append(&path, linea_consola[2]);

	FILE *archivo = fopen(path, "r");
	if (archivo == NULL)
	{
		puts("ERROR");
		perror("Error al abrir fichero.txt");
	}
	free(path);

	// char cadena[50]; /* Un array lo suficientemente grande como para guardar la línea más larga del fichero */
	//strcpy(cadena,"");
	//char *linea_file = malloc(50);
	t_paquete *paquete = crear_paquete(INICIAR_PATOTA);

	int id = ID_PATOTA;

	log_info(logger,"%d %d",id,atoi(linea_consola[1]));
	agregar_a_paquete(paquete, string_itoa(id), sizeof(id));
	agregar_a_paquete(paquete, linea_consola[1], strlen(linea_consola[1]) + 1);

	char *line_buf = NULL;
	size_t line_buf_size = 0;
	ssize_t line_size;

	line_size = getline(&line_buf, &line_buf_size, archivo);

	while (line_size >= 0)
	{
	char* tarea_trim = malloc(line_size+2);
		strcpy(tarea_trim,line_buf);

		if ( tarea_trim[line_size-1] != '\n') 
		{
			tarea_trim[line_size]='-';
			tarea_trim[line_size+1]='\0';
		}else
		{
			tarea_trim[line_size-1]='-';
			tarea_trim[line_size]='\0';
		}
		
		 
		log_info(logger,"%s",tarea_trim);
		agregar_a_paquete(paquete, tarea_trim, strlen(tarea_trim) + 1);
		line_size = getline(&line_buf, &line_buf_size, archivo);
	free(tarea_trim);
	}

	char *posiciones_linea = string_new();
	for (int i = 3; linea_consola[i] != NULL; i++)
	{
		string_append(&posiciones_linea, linea_consola[i]);

		if (linea_consola[i + 1] != NULL)
		{
			string_append(&posiciones_linea, ";");
		}
	}

	agregar_a_paquete(paquete, posiciones_linea, strlen(posiciones_linea) + 1);
	free(posiciones_linea);
	// log_info(logger, "Enviando tareas a MI-RAM");
	enviar_paquete(paquete, conexion);
	eliminar_paquete(paquete);
	fclose(archivo);
}

void recepcionar_patota(char **linea_consola)
{
	int posx, posy;
	char **posiciones;
	int tripulantes = atoi(linea_consola[1]);
	int id_patota = ID_PATOTA;

	for (uint32_t i = 1; i <= tripulantes; i++)
	{
		// log_info(logger, "POSICIONANDOME EN NEW TRIPU %d DE PATOTA %d", i, id_patota);
		posx = 0;
		posy = 0;
		if (linea_consola[i + 2] != NULL)
		{ //las posiciones comienzan desde el argumento 3 en adelante.
			posiciones = string_split(linea_consola[i + 2], "|");
			posx = atoi(posiciones[0]);
			posy = atoi(posiciones[1]);
		}

		//INICIARLIZAR TRIPULANTE
		t_tripulante *tripulante = crear_tripulante(id_patota, posx, posy, i);

		add_queue(_NEW_, tripulante);
	}
	free(posiciones);
	pthread_mutex_unlock(&mutex_planificacion);
}

void iterator_lines_free(char *string)
{
	free(string);
}

void *recibir_mensaje_de_RAM(int socket_cliente, t_log *logger, int *direccion_size)
{
	// int size;
	char *buffer = recibir_buffer(direccion_size, socket_cliente);
	return buffer;
}

t_tarea *recibir_tarea_de_RAM(int socket)
{
	t_paquete *paquete = malloc(sizeof(t_paquete));
	paquete->buffer = malloc(sizeof(t_buffer));
	// Primero recibimos el codigo de operacion
	recv(socket, &(paquete->codigo_operacion), sizeof(uint32_t), 0);
	// Después ya podemos recibir el buffer. Primero su tamaño seguido del contenido
	recv(socket, &(paquete->buffer->size), sizeof(uint32_t), 0);
	paquete->buffer->stream = malloc(paquete->buffer->size);
	recv(socket, paquete->buffer->stream, paquete->buffer->size, 0);

	char **tarea_completa_array = string_split(paquete->buffer->stream, ";");
	char **tarea;

	t_tarea *tarea_recibida = malloc(sizeof(t_tarea));
	log_info(logger,"tarea recibida %s",paquete->buffer->stream);
	if (strcmp(paquete->buffer->stream, "NULL") == 0)
	{
		tarea_recibida->accion = "NULL";
		return tarea_recibida;
	}

	if (string_contains(tarea_completa_array[0], " "))
	{
		tarea = string_split(tarea_completa_array[0], " ");
		tarea_recibida->accion = malloc(strlen(tarea[0]));
		strcpy(tarea_recibida->accion, tarea[0]);
		tarea_recibida->parametro = atoi(tarea[1]);
	}
	else
	{
		tarea_recibida->accion = malloc(strlen(tarea_completa_array[0])+1);
		strcpy(tarea_recibida->accion, tarea_completa_array[0]);
		tarea_recibida->parametro = -1;
	}

	tarea_recibida->posicion_x = atoi(tarea_completa_array[1]);
	tarea_recibida->posicion_y = atoi(tarea_completa_array[2]);
	tarea_recibida->tiempo = atoi(tarea_completa_array[3]);

	string_iterate_lines(tarea_completa_array, iterator_lines_free);
	free(tarea_completa_array);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);

	return tarea_recibida;
}

void add_queue(int lista, t_tripulante *tripulante)
{
	switch (lista)
	{
	case _NEW_:;
		tripulante->estado = 'N';
		pthread_mutex_lock(&mutex_lista_new);
		list_add(NEW, tripulante);
		pthread_mutex_unlock(&mutex_lista_new);
		break;
	case _READY_:;
		tripulante->estado = 'R';
		pthread_mutex_lock(&mutex_lista_ready);
		enviar_nuevo_estado_a_ram(tripulante);
		list_add(READY, tripulante);
		pthread_mutex_unlock(&mutex_lista_ready);
		break;
	case _BLOCKED_:;
		tripulante->estado = 'B';
		pthread_mutex_lock(&mutex_lista_blocked);
		enviar_nuevo_estado_a_ram(tripulante);
		list_add(BLOCKED, tripulante);
		pthread_mutex_unlock(&mutex_lista_blocked);
		break;
	case _BLOCKED_EMERGENCY_:;
		tripulante->estado = 'S';
		enviar_nuevo_estado_a_ram(tripulante);
		list_add(BLOCKED_EMERGENCY, tripulante);
		break;
	case _EXIT_:;

		pthread_mutex_lock(&mutex_lista_exit);
		// Agregar Funcion Expulsar
		enviar_expulsar_tripulante_a_ram(tripulante);
		controlar_forma_de_salida(tripulante);
		tripulante->estado = 'F';
		list_add(EXIT, tripulante);
		pthread_mutex_unlock(&mutex_lista_exit);
		break;
	case _EXEC_:;
		tripulante->estado = 'E';
		pthread_mutex_lock(&mutex_lista_exec);
		enviar_nuevo_estado_a_ram(tripulante);
		list_add(EXEC, tripulante);
		pthread_mutex_unlock(&mutex_lista_exec);
		break;
	default:
		break;
	}
}

void controlar_forma_de_salida(t_tripulante* t){
	if(t->estado== 'E' || t->estado=='B'){
	if(!string_equals_ignore_case(t->tarea->accion,"NULL")){
		strcpy(t->tarea->accion,"NULL");
		t->posicion_x = t->tarea->posicion_x;
		t->posicion_y = t->tarea->posicion_y;
		t->QUANTUM_ACTUAL = 0;
		pthread_mutex_lock(&mutex_movimiento);
		t->tarea->tiempo = 0;
		pthread_mutex_unlock(&mutex_movimiento);
	}
	}
}

////////////////////////////////////////////////////atender_accion para hilo

void atender_accion_de_consola(char *linea_consola)
{

	char **array_parametros = string_split(linea_consola, " ");
	free(linea_consola);
	switch (get_diccionario_accion(array_parametros[0]))
	{
	case INICIAR_PATOTA_:;
		ID_PATOTA = ID_PATOTA + 1;
		// SEND TAREAS A RAM
		int conexion = crear_conexion(IP_MI_RAM_HQ, PUERTO_MI_RAM_HQ);
		enviar_tareas_a_RAM(conexion, array_parametros);
		liberar_conexion(conexion);
		// SEND TAREAS A RAM
		recepcionar_patota(array_parametros);
		string_iterate_lines(array_parametros, iterator_lines_free);
		free(array_parametros);
		// int conexion_imongo = crear_conexion(IP_I_MONGO_STORE, PUERTO_I_MONGO_STORE);
		// enviar_mensaje_and_codigo_op("GENERAR_OXIGENO 4",MENSAJE,conexion_imongo);
		// liberar_conexion(conexion_imongo);
		break;
	case INICIAR_PLANIFICACION:
		continuar_planificacion();
		iniciar_planificacion();
		break;
	case LISTAR_TRIPULANTE:

		list_iterate(NEW, (void *)iterator);
		list_iterate(EXIT, (void *)iterator);
		list_iterate(BLOCKED, (void *)iterator);
		list_iterate(READY, (void *)iterator);
		list_iterate(EXEC, (void *)iterator);

		break;
	case EXPULSAR_TRIPULANTE_:
		mover_tripulante_entre_listas_si_existe(_NEW_, _EXIT_, atoi(array_parametros[1]), atoi(array_parametros[2]));
		mover_tripulante_entre_listas_si_existe(_READY_, _EXIT_, atoi(array_parametros[1]), atoi(array_parametros[2]));
		mover_tripulante_entre_listas_si_existe(_EXEC_, _EXIT_, atoi(array_parametros[1]), atoi(array_parametros[2]));
		mover_tripulante_entre_listas_si_existe(_BLOCKED_, _EXIT_, atoi(array_parametros[1]), atoi(array_parametros[2]));
		mover_tripulante_entre_listas_si_existe(_NEW_,_EXIT_, atoi(array_parametros[1]), atoi(array_parametros[2]));
		break;
	case OBTENER_BITACORA:
		//solicitar a IMONGOSTORE la bitacora del tripulante
		
		break;
	case PAUSAR_PLANIFICACION:
		pausar_planificacion();
		break;
	default:
		log_info(logger, "COMANDO NO DISPONIBLE");
		break;
	}
}

void iniciar_planificacion()
{
	if (ALGORITMO == FIFO)
	{log_info(logger,"Planificador FIFO");

		while (1)
		{
			pthread_mutex_lock(&mutex_planificacion);
			//sem_post(&sem_exe);
			int tripulantes_en_new = list_size(NEW);
			pthread_t hilo[tripulantes_en_new];
			for (size_t i = 1; i <= tripulantes_en_new; i++)
			{
				t_tripulante *tripulante = list_remove(NEW, 0);
				pthread_create(&hilo[i], NULL, (void *)planificar_FIFO, tripulante);
			}


			for (uint32_t j = 1; j <= tripulantes_en_new; j++)
			{
				pthread_detach(hilo[j]);
			}
		}
	}

	if (ALGORITMO == RR)
	{	log_info(logger,"Planificador RR Q=%d",QUANTUM);
		while (1)
		{
			pthread_mutex_lock(&mutex_planificacion);
			int tripulantes_en_new = list_size(NEW);
			pthread_t hilo[tripulantes_en_new];

			for (size_t i = 1; i <= tripulantes_en_new; i++)
			{
				t_tripulante *tripulante = list_remove(NEW, 0);
				pthread_create(&hilo[i], NULL, (void *)planificar_RR, tripulante);
			}

			for (uint32_t j = 1; j <= tripulantes_en_new; j++)
			{
				pthread_detach(hilo[j]);
			}
		}
	}
}

void atender_sabotaje()
{
	
	while (1)
	{	
		log_info(logger, "Conectandome con IMONGOSTORE por situaciones de sabotaje");
		// int socket_conexion = crear_conexion(IP_I_MONGO_STORE, PUERTO_I_MONGO_STORE);
		int socket_conexion = crear_conexion(IP_I_MONGO_STORE, PUERTO_I_MONGO_STORE);
		log_info(logger, "Socket %d id_sabotaje %d", socket_conexion, SABOTAJE);
		int *f = (int *)SABOTAJE;
		send(socket_conexion, &f, sizeof(int), 0);
		int direccion_size;
		recibir_operacion(socket_conexion);
		char* coordenada_sabotaje = (char*) recibir_buffer(&direccion_size,socket_conexion);
		log_info(logger,"%s tamanio %d",coordenada_sabotaje,direccion_size);
		// liberar_conexion(socket_conexion);
		char** coor = string_split(coordenada_sabotaje,"|");
		int sabotaje_x=atoi(coor[0]);
		int sabotaje_y=atoi(coor[1]);
		free(coordenada_sabotaje);


		activar_sabotaje();
		log_info(logger,"Buscando un tripulante para resolver sabotaje en %d %d",sabotaje_x,sabotaje_y);
		agregar_tripulantes_a_BLOCKED_EMERGENCY_en_sabotaje();
		resolver_sabotaje_por_tripulante_mas_cercano_a_posicion(sabotaje_x, sabotaje_y);

		//dar aviso a IMONGO STORE de sabotaje resuelto
		// int socket = crear_conexion(IP_I_MONGO_STORE, PUERTO_I_MONGO_STORE);
				// int *a = (int *)SABOTAJE_RESUELTO;
		char* respuesta_de_sabotaje = malloc(strlen("SABOTAJE_OK")+1);
		strcpy(respuesta_de_sabotaje,"SABOTAJE_OK");
		enviar_mensaje(respuesta_de_sabotaje,socket_conexion);
		// send(socket, &a, sizeof(int), 0);

		//dar aviso a IMONGO STORE de sabotaje resuelto
		free(respuesta_de_sabotaje);
		list_clean(BLOCKED_EMERGENCY);
		volver_a_actividad();
		desactivar_sabotaje();
		liberar_conexion(socket_conexion);
	}
}

void verificar_existencia_de_sabotaje()
{
	if (EXISTE_SABOTAJE)
		pthread_cond_wait(&semaforo_sabotaje, &mutex_sabotaje);
}

void activar_sabotaje()
{
	EXISTE_SABOTAJE = true;
}

void desactivar_sabotaje()
{
	EXISTE_SABOTAJE = false;
	pthread_mutex_unlock(&mutex_sabotaje);
	pthread_cond_signal(&semaforo_sabotaje);
}

///implementacion de pausar planificacion
void verificar_existencia_de_pausado()
{
	if (PAUSEAR_PLANIFICACION)
	{
		pthread_cond_wait(&condicion_pausear_planificacion, &mutex_pausar);
	}
}

void pausar_planificacion()
{
	PAUSEAR_PLANIFICACION = true;
}

void continuar_planificacion()
{
	PAUSEAR_PLANIFICACION = false;
	pthread_mutex_unlock(&mutex_pausar);
	pthread_cond_signal(&condicion_pausear_planificacion);
}

void agregar_tripulantes_a_BLOCKED_EMERGENCY_en_sabotaje()
{

	for (size_t i = 0; i < list_size(EXEC); i++)
	{
		t_tripulante* tripulante_e = list_get(EXEC,i);
		tripulante_e->estado = 'S';
		
		//enviar_mensajes_en_sabotaje_a_imongo_store_para_BITACORA(tripulante_e,"Se corre en pánico hacia la ubicación del sabotaje");
		list_add(BLOCKED_EMERGENCY,tripulante_e);

	}

	for (size_t i = 0; i < list_size(READY); i++)
	{
		t_tripulante *tripulante_r = list_get(READY, i);
		tripulante_r->estado = 'S';
		//enviar_mensajes_en_sabotaje_a_imongo_store_para_BITACORA(tripulante_r,"Se corre en pánico hacia la ubicación del sabotaje");
		add_queue(_BLOCKED_EMERGENCY_, tripulante_r);
	}
}

void volver_a_actividad(){

	for (size_t i = 0; i < list_size(EXEC); i++)
	{
		t_tripulante* tripu = list_get(EXEC,i);
		tripu->estado = 'E';
	}

		for (size_t e = 0; e < list_size(READY); e++)
	{
		t_tripulante* tripu = list_get(READY,e);
		tripu->estado = 'R';
	}
}

void resolver_sabotaje_por_tripulante_mas_cercano_a_posicion(int x, int y)
{

	//aca deberia sacar al tripulante mas cercano a la posicion (x,y)
	
	t_tripulante *tripulante = devolver_el_tripulante_mas_cercano_a_la_emergencia(x,y);
				log_info(logger,"%s","Lo copie de la cola de emergencia para mover hacia la tarea de sabotaje");
iterator(tripulante);

	moverme_hacia_tarea_en_sabotaje(tripulante,x,y);

	//realizar tarea
	for (size_t i = 0; i < DURACION_SABOTAJE; i++)
	{
		loggear_linea();
		sleep(1);
	}
		log_info(logger,"%s","Se resolvio el sabotaje");
		iterator(tripulante);
}

t_tripulante* devolver_el_tripulante_mas_cercano_a_la_emergencia(int sabotaje_pos_x, int sabotaje_pos_y)
{
    // list_sort(BLOCKED_EMERGENCY,orden_lista_BLOCKED_EMERGENCY);
    t_list_iterator* list_iterator_sabotaje = list_iterator_create(BLOCKED_EMERGENCY);

    t_tripulante* tripulante_mas_cercano;
    int menor_distancia = 9999;
    while(list_iterator_has_next(list_iterator_sabotaje))
    {
        t_tripulante* tripulante_en_emergencia = (t_tripulante*) list_iterator_next(list_iterator_sabotaje);
        int diferencia_x = abs(tripulante_en_emergencia->posicion_x - sabotaje_pos_x);
        int diferencia_y = abs(tripulante_en_emergencia->posicion_y - sabotaje_pos_y);

        int distancia = diferencia_x + diferencia_y;

        if ( distancia < menor_distancia)
        {
            menor_distancia = distancia;
            tripulante_mas_cercano = tripulante_en_emergencia;
        }

    }
    list_iterator_destroy(list_iterator_sabotaje);

    return tripulante_mas_cercano;
}

void sacar_tripulantes_de_BLOCKED_EMERGENCY()
{

	int tamanio_lista = list_size(BLOCKED_EMERGENCY);
	for (size_t i = 0; i < tamanio_lista; i++)
	{
		list_remove(BLOCKED_EMERGENCY, 0);
	}
}

void moverme_hacia_tarea_en_sabotaje(t_tripulante *tripulante, int x, int y)
{
	int socket;
	while (tripulante->posicion_x < x)
	{
		log_info(logger, "%d-%d me muevo en sabotaje de (%d,%d) a (%d,%d)",
				 tripulante->puntero_pcb, tripulante->tid, tripulante->posicion_x, tripulante->posicion_y,
				 tripulante->posicion_x + 1, tripulante->posicion_y);
		tripulante->posicion_x = tripulante->posicion_x + 1;
		sleep(1);
		socket = crear_conexion(IP_MI_RAM_HQ, PUERTO_MI_RAM_HQ);
		enviar_posicion_a_ram(tripulante, (int)socket);
		liberar_conexion(socket);
	}

	while (tripulante->posicion_x > x)
	{
		log_info(logger, "%d-%d me muevo en sabotaje de (%d,%d) a (%d,%d)",
				 tripulante->puntero_pcb, tripulante->tid, tripulante->posicion_x, tripulante->posicion_y,
				 tripulante->posicion_x - 1, tripulante->posicion_y);
		tripulante->posicion_x = tripulante->posicion_x - 1;
		sleep(1);
		socket = crear_conexion(IP_MI_RAM_HQ, PUERTO_MI_RAM_HQ);
		enviar_posicion_a_ram(tripulante, (int)socket);
		liberar_conexion(socket);
	}

	while (tripulante->posicion_y < y)
	{
		log_info(logger, "%d-%d me muevo de (%d,%d) a (%d,%d)",
				 tripulante->puntero_pcb, tripulante->tid, tripulante->posicion_x, tripulante->posicion_y,
				 tripulante->posicion_x, tripulante->posicion_y + 1);
		tripulante->posicion_y = tripulante->posicion_y + 1;
		sleep(1);
		socket = crear_conexion(IP_MI_RAM_HQ, PUERTO_MI_RAM_HQ);
		enviar_posicion_a_ram(tripulante, (int)socket);
		liberar_conexion(socket);
	}

	while (tripulante->posicion_y > y)
	{
		log_info(logger, "%d-%d me muevo de (%d,%d) a (%d,%d)",
				 tripulante->puntero_pcb, tripulante->tid, tripulante->posicion_x, tripulante->posicion_y,
				 tripulante->posicion_x, tripulante->posicion_y - 1);
		tripulante->posicion_y = tripulante->posicion_y - 1;
		sleep(1);
		socket = crear_conexion(IP_MI_RAM_HQ, PUERTO_MI_RAM_HQ);
		enviar_posicion_a_ram(tripulante, (int)socket);
		liberar_conexion(socket);
	}

	log_info(logger, "%d-%d llegué al sabotaje", tripulante->puntero_pcb, tripulante->tid);
}

int obtener_algoritmo(char *algoritmo_de_planificacion)
{

	if (string_equals_ignore_case(algoritmo_de_planificacion, "FIFO"))
	{
		return FIFO;
	}
	else
	{
		return RR;
	}
}

////envios de mensajes a RAM
void enviar_posicion_a_ram(t_tripulante *tripulante, int socket)
{
	t_paquete *paquete = crear_paquete(RECIBIR_LA_UBICACION_DEL_TRIPULANTE);

	char *patota = string_itoa(tripulante->puntero_pcb);
	char *trip = string_itoa(tripulante->tid);
	char *pos_x = string_itoa(tripulante->posicion_x);
	char *pos_y = string_itoa(tripulante->posicion_y);

	agregar_a_paquete(paquete, patota, strlen(patota) + 1);
	agregar_a_paquete(paquete, trip, strlen(trip) + 1);
	agregar_a_paquete(paquete, pos_x, strlen(pos_x) + 1);
	agregar_a_paquete(paquete, pos_y, strlen(pos_y) + 1);

	enviar_paquete(paquete, socket);

	free(patota);
	free(trip);
	free(pos_x);
	free(pos_y);
	eliminar_paquete(paquete);
}

void enviar_nuevo_estado_a_ram(t_tripulante *tripulante)
{
	int socket = crear_conexion(IP_MI_RAM_HQ, PUERTO_MI_RAM_HQ);
	t_paquete *paquete = crear_paquete(ACTUALIZAR_ESTADO);

	char *patota = string_itoa(tripulante->puntero_pcb);
	char *trip = string_itoa(tripulante->tid);
	char *estado_ = malloc(2);
	estado_[0] = tripulante->estado;
	estado_[1] = '\0';
	agregar_a_paquete(paquete, patota, strlen(patota) + 1);
	agregar_a_paquete(paquete, trip, strlen(trip) + 1);
	agregar_a_paquete(paquete, estado_, strlen(estado_) + 1);

	enviar_paquete(paquete, socket);


	free(patota);
	free(trip);
	free(estado_);
	eliminar_paquete(paquete);
	liberar_conexion(socket);
}

void enviar_expulsar_tripulante_a_ram(t_tripulante *tripulante)
{
	pthread_mutex_lock(&mutex_mostrar_por_consola);
	int socket = crear_conexion(IP_MI_RAM_HQ, PUERTO_MI_RAM_HQ);
	t_paquete *paquete = crear_paquete(EXPULSAR_TRIPULANTE);

	char *patota = string_itoa(tripulante->puntero_pcb);
	char *trip = string_itoa(tripulante->tid);
	agregar_a_paquete(paquete, patota, strlen(patota) + 1);
	agregar_a_paquete(paquete, trip, strlen(trip) + 1);

	enviar_paquete(paquete, socket);

	free(patota);
	free(trip);
	eliminar_paquete(paquete);
	liberar_conexion(socket);
	pthread_mutex_unlock(&mutex_mostrar_por_consola);
}

void buscar_tarea_a_RAM(t_tripulante *tripulante)
{
	pthread_mutex_lock(&mutex_mostrar_por_consola);
	int socket = crear_conexion(IP_MI_RAM_HQ, PUERTO_MI_RAM_HQ);
	t_paquete *paquete = crear_paquete(ENVIAR_PROXIMA_TAREA);

	char *patota = string_itoa(tripulante->puntero_pcb);
	char *trip = string_itoa(tripulante->tid);

	agregar_a_paquete(paquete, patota, strlen(patota) + 1);
	agregar_a_paquete(paquete, trip, strlen(trip) + 1);

	enviar_paquete(paquete, socket);
	
	tripulante->tarea = recibir_tarea_de_RAM(socket);

	free(patota);
	free(trip);
	eliminar_paquete(paquete);
	liberar_conexion(socket);
	pthread_mutex_unlock(&mutex_mostrar_por_consola);
}

void loggear_linea()
{
    log_info(logger,"-------------------------------------------------------------");
}

//mensajes a IMONGOSTORE
/*
 ●  Se mueve de X|Y a X’|Y’
 ● Comienza ejecución de tarea X
 ● Se finaliza la tarea X
 ● Se corre en pánico hacia la ubicación del sabotaje
 ● Se resuelve el sabotaje
 */

void enviar_movimiento_a_imongo_store_para_BITACORA(t_tripulante *tripulante){
/*
mensaje: "Se mueve de X|Y a X’|Y’"
 */

char* tripulante_pos_x =string_itoa(tripulante->posicion_x);
char* tripulante_pos_y =string_itoa(tripulante->posicion_y);

char* tripu_posicion_x = strcat(tripulante_pos_x,"|");
char* coordenada_tripu = strcat(tripu_posicion_x,tripulante_pos_y);

char* tarea_pos_x =string_itoa(tripulante->tarea->posicion_x);
char* tarea_pos_y =string_itoa(tripulante->tarea->posicion_y);

char* tarea_posicion_x = strcat(tarea_pos_x,"|");
char* coordenada_tarea = strcat(tarea_posicion_x,tarea_pos_y);

char* mensaje = malloc(strlen("Se mueve de ")+strlen(coordenada_tripu)+strlen(coordenada_tarea)
						+strlen(" a ")+1+1);
strcpy(mensaje,"Se mueve de ");
strcat(mensaje,coordenada_tripu);
strcat(mensaje," a ");
strcat(mensaje,coordenada_tarea);
strcat(mensaje,"\n");

t_bitacora* bitacora = malloc(sizeof(t_bitacora)); 
bitacora->id_tripulante = tripulante->tid;
bitacora->id_patota = tripulante->puntero_pcb;
bitacora->length_mensaje = strlen(mensaje);
bitacora->mensaje = mensaje;

int conexion = crear_conexion(IP_I_MONGO_STORE, PUERTO_I_MONGO_STORE);
log_info(logger,bitacora->mensaje);
enviar_bitacora(bitacora,conexion);

free(tripulante_pos_x);
free(tripulante_pos_y);
free(tarea_pos_x);
free(tarea_pos_y);
liberar_conexion(conexion);
}

void enviar_comienzo_o_finalizacion_de_tarea_a_imongo_store_para_BITACORA(t_tripulante *tripulante,char* mensaje){
/*
mensaje:"Comienza ejecución de tarea X" o "Se finaliza la tarea X"
 */
char* msj = malloc(strlen(mensaje)+strlen(tripulante->tarea->accion)+1+1);
strcpy(msj,mensaje);
strcat(msj,tripulante->tarea->accion);
strcat(msj,"\n");


t_bitacora* bitacora = malloc(sizeof(t_bitacora)); 
bitacora->id_tripulante = tripulante->tid;
bitacora->id_patota = tripulante->puntero_pcb;
bitacora->length_mensaje = strlen(msj);
bitacora->mensaje = msj;

int conexion = crear_conexion(IP_I_MONGO_STORE, PUERTO_I_MONGO_STORE);
log_info(logger,bitacora->mensaje);
enviar_bitacora(bitacora,conexion);
free(msj);
free(bitacora);
liberar_conexion(conexion);
}

void enviar_mensajes_en_sabotaje_a_imongo_store_para_BITACORA(t_tripulante *tripulante,char* mensaje){
/*
mensaje: "Se corre en pánico hacia la ubicación del sabotaje"
y " Se resuelve el sabotaje"
 */

t_bitacora* bitacora = malloc(sizeof(t_bitacora)); 
bitacora->id_tripulante = tripulante->tid;
bitacora->id_patota = tripulante->puntero_pcb;
bitacora->length_mensaje = strlen(mensaje);
bitacora->mensaje = malloc(strlen(mensaje)+1);
bitacora->mensaje = mensaje;

int conexion = crear_conexion(IP_I_MONGO_STORE, PUERTO_I_MONGO_STORE);
log_info(logger,bitacora->mensaje);
enviar_bitacora(bitacora,conexion);
free(bitacora->mensaje);
free(bitacora);
liberar_conexion(conexion);
}

void enviar_tarea_de_ES_a_imongostore(t_tripulante *tripulante){

char* parametro_de_tarea = string_itoa(tripulante->tarea->parametro);

char* tarea_espacio_parametro = malloc(strlen(tripulante->tarea->accion)+1
							+strlen(parametro_de_tarea)+1+1);

strcpy(tarea_espacio_parametro,tripulante->tarea->accion);
strcat(tarea_espacio_parametro," ");
strcat(tarea_espacio_parametro,parametro_de_tarea);

int conexion = crear_conexion(IP_I_MONGO_STORE, PUERTO_I_MONGO_STORE);
log_info(logger,tarea_espacio_parametro);
enviar_mensaje_and_codigo_op(tarea_espacio_parametro,MENSAJE,conexion);
free(parametro_de_tarea);
free(tarea_espacio_parametro);
liberar_conexion(conexion);
}

