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
	// pthread_t hiloEscuchaSabotaje;
	// pthread_create(&hiloEscuchaSabotaje, NULL, (void *)atender_sabotaje, NULL);

	pthread_t hilo_recepcionista;
	while (1)
	{
		char *retorno_consola = readline(">");
		pthread_create(&hilo_recepcionista, NULL, (void *)atender_accion_de_consola, retorno_consola);
	}
	pthread_join(hilo_recepcionista, NULL);
	// pthread_join(hiloEscuchaSabotaje, NULL);

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
	}

	if (list_any_satisfy((t_list*)obtener_lista(lista_origen), _el_tripulante_que_limpio))
	{

		add_queue(list_destino, list_remove_by_condition((t_list*)obtener_lista(lista_origen), _el_tripulante_que_limpio));
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
	void *magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento += paquete->buffer->size;

	send(socket_cliente, magic, bytes, 0); //envia y espera la respuesta

	free(magic);
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
	string_append(&path, "/home/utnso/workspace/tp-2021-1c-Quinta-Recursada/discordiador/tareas/");
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

	agregar_a_paquete(paquete, string_itoa(id), sizeof(id));
	agregar_a_paquete(paquete, linea_consola[1], strlen(linea_consola[1]) + 1);

	char *line_buf = NULL;
	size_t line_buf_size = 0;
	ssize_t line_size;

	line_size = getline(&line_buf, &line_buf_size, archivo);

	while (line_size >= 0)
	{
		strcat(line_buf, "-");
		agregar_a_paquete(paquete, line_buf, strlen(line_buf) + 1);
		line_size = getline(&line_buf, &line_buf_size, archivo);
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
		tarea_recibida->accion = malloc(strlen(tarea_completa_array[0]));
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
		tripulante->estado = 'F';
		pthread_mutex_lock(&mutex_lista_exit);
		// Agregar Funcion Expulsar
		controlar_forma_de_salida(tripulante);
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
	if((strcmp(&t->estado, "E") == 0) ||(strcmp(&t->estado, "B") == 0)){
	if(!(string_equals_ignore_case(t->tarea->accion,"NULL"))==0){
		strcpy(t->tarea->accion,"NULL");
		t->posicion_x = t->tarea->posicion_x;
		t->posicion_y = t->tarea->posicion_y;
		t->tarea->tiempo = 0;
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
		//SEND TAREAS A RAM
		int conexion = crear_conexion(IP_MI_RAM_HQ, PUERTO_MI_RAM_HQ);
		enviar_tareas_a_RAM(conexion, array_parametros);
		liberar_conexion(conexion);
		//SEND TAREAS A RAM
		recepcionar_patota(array_parametros);
		string_iterate_lines(array_parametros, iterator_lines_free);
		free(array_parametros);
		break;
	case INICIAR_PLANIFICACION:
		continuar_planificacion();
		iniciar_planificacion();
		break;
	case LISTAR_TRIPULANTE:
		list_iterate(NEW, (void *)iterator);
		list_iterate(READY, (void *)iterator);
		list_iterate(BLOCKED, (void *)iterator);
		list_iterate(EXEC, (void *)iterator);
		list_iterate(EXIT, (void *)iterator);
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
	{
		while (1)
		{
			pthread_mutex_lock(&mutex_planificacion);
			//sem_post(&sem_exe);
			int tripulantes_en_new = list_size(NEW);
			pthread_t hilo[tripulantes_en_new];
			pthread_t planificador_ES;

			for (size_t i = 1; i <= tripulantes_en_new; i++)
			{
				t_tripulante *tripulante = list_remove(NEW, 0);
				pthread_create(&hilo[i], NULL, (void *)planificar_FIFO, tripulante);
			}

			pthread_create(&planificador_ES, NULL, (void *)entrada_salida, NULL);
			pthread_detach(planificador_ES);

			for (uint32_t j = 1; j <= tripulantes_en_new; j++)
			{
				pthread_detach(hilo[j]);
			}
		}
	}

	if (ALGORITMO == RR)
	{
		while (1)
		{
			pthread_mutex_lock(&mutex_planificacion);
			int tripulantes_en_new = list_size(NEW);
			pthread_t hilo[tripulantes_en_new];
			pthread_t planificador_ES;

			for (size_t i = 1; i <= tripulantes_en_new; i++)
			{
				t_tripulante *tripulante = list_remove(NEW, 0);
				pthread_create(&hilo[i], NULL, (void *)planificar_RR, tripulante);
			}

			pthread_create(&planificador_ES, NULL, (void *)entrada_salida, NULL);
			pthread_detach(planificador_ES);

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
		int socket_conexion = crear_conexion(IP_I_MONGO_STORE, PUERTO_I_MONGO_STORE);
		// log_info(logger, "Socket %d id_sabotaje %d", socket_conexion, SABOTAJE);

		// int *f = (int *)SABOTAJE;
	
		// send(socket_conexion, &f, sizeof(int), 0);

		// int direccion_size;
		// char* mensaje = recibir_mensaje(socket_conexion,logger,direccion_size);

		// log_info(logger, "Señal del SABOTAJE recibida %s",mensaje);
		// free(mensaje);
		
		activar_sabotaje();
		agregar_tripulantes_a_BLOCKED_EMERGENCY_en_sabotaje();
		resolver_sabotaje_por_tripulante_mas_cercano_a_posicion(3, 3);
		desactivar_sabotaje();
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
		t_tripulante *tripulante_e = list_get(EXEC, i);
		add_queue(_BLOCKED_EMERGENCY_, tripulante_e);
	}

	for (size_t i = 0; i < list_size(READY); i++)
	{
		t_tripulante *tripulante_r = list_get(READY, i);
		add_queue(_BLOCKED_EMERGENCY_, tripulante_r);
	}
}

void resolver_sabotaje_por_tripulante_mas_cercano_a_posicion(int x, int y)
{

	//aca deberia sacar al tripulante mas cercano a la posicion (x,y)
	t_tripulante *tripulante = list_remove(BLOCKED_EMERGENCY, 0);

	moverme_hacia_tarea_en_sabotaje(tripulante, 5, 5);

	//realizar tarea
	int duracion_sabotaje = 5;
	sleep(duracion_sabotaje); //variable GLOBAL que viene por config

	list_add(BLOCKED_EMERGENCY, tripulante);
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
	int socket = crear_conexion(IP_MI_RAM_HQ, PUERTO_MI_RAM_HQ);
	while (tripulante->posicion_x < x)
	{
		// log_info(logger, "%d-%d me muevo de (%d,%d) a (%d,%d)",
		// 		 tripulante->puntero_pcb, tripulante->tid, tripulante->posicion_x, tripulante->posicion_y,
		// 		 tripulante->posicion_x + 1, tripulante->posicion_y);
		tripulante->posicion_x = tripulante->posicion_x + 1;
		sleep(1);
		enviar_posicion_a_ram(tripulante, (int)socket);
		socket = crear_conexion(IP_MI_RAM_HQ, PUERTO_MI_RAM_HQ);
	}

	while (tripulante->posicion_x > x)
	{
		// log_info(logger, "%d-%d me muevo de (%d,%d) a (%d,%d)",
		// 		 tripulante->puntero_pcb, tripulante->tid, tripulante->posicion_x, tripulante->posicion_y,
		// 		 tripulante->posicion_x - 1, tripulante->posicion_y);
		tripulante->posicion_x = tripulante->posicion_x - 1;
		sleep(1);
		enviar_posicion_a_ram(tripulante, (int)socket);
		socket = crear_conexion(IP_MI_RAM_HQ, PUERTO_MI_RAM_HQ);
	}

	while (tripulante->posicion_y < y)
	{
		// log_info(logger, "%d-%d me muevo de (%d,%d) a (%d,%d)",
		// 		 tripulante->puntero_pcb, tripulante->tid, tripulante->posicion_x, tripulante->posicion_y,
		// 		 tripulante->posicion_x, tripulante->posicion_y + 1);
		tripulante->posicion_y = tripulante->posicion_y + 1;
		sleep(1);

		enviar_posicion_a_ram(tripulante, (int)socket);
		socket = crear_conexion(IP_MI_RAM_HQ, PUERTO_MI_RAM_HQ);
	}

	while (tripulante->posicion_y > y)
	{
		// log_info(logger, "%d-%d me muevo de (%d,%d) a (%d,%d)",
		// 		 tripulante->puntero_pcb, tripulante->tid, tripulante->posicion_x, tripulante->posicion_y,
		// 		 tripulante->posicion_x, tripulante->posicion_y - 1);
		tripulante->posicion_y = tripulante->posicion_y - 1;
		sleep(1);
		enviar_posicion_a_ram(tripulante, (int)socket);
		socket = crear_conexion(IP_MI_RAM_HQ, PUERTO_MI_RAM_HQ);
	}
	liberar_conexion(socket);

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

	free(patota);
	free(trip);
	free(pos_x);
	free(pos_y);

	enviar_paquete(paquete, socket);
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

	free(patota);
	free(trip);
	free(estado_);

	enviar_paquete(paquete, socket);

	eliminar_paquete(paquete);
	liberar_conexion(socket);
}

void enviar_expulsar_tripulante_a_ram(t_tripulante *tripulante)
{
	int socket = crear_conexion(IP_MI_RAM_HQ, PUERTO_MI_RAM_HQ);
	t_paquete *paquete = crear_paquete(EXPULSAR_TRIPULANTE);

	agregar_a_paquete(paquete, string_itoa(tripulante->puntero_pcb), strlen(string_itoa(tripulante->puntero_pcb) + 1));
	agregar_a_paquete(paquete, string_itoa(tripulante->tid), strlen(string_itoa(tripulante->tid) + 1));

	enviar_paquete(paquete, socket);

	eliminar_paquete(paquete);
	liberar_conexion(socket);
}

void buscar_tarea_a_RAM(t_tripulante *tripulante)
{
	pthread_mutex_lock(&mutex_mostrar_por_consola);
	int socket = crear_conexion(IP_MI_RAM_HQ, PUERTO_MI_RAM_HQ);
	t_paquete *paquete = crear_paquete(ENVIAR_PROXIMA_TAREA);

	// char *patota = string_itoa(tripulante->puntero_pcb);
	char *trip = string_itoa(tripulante->tid);

	// agregar_a_paquete(paquete, patota, strlen(patota) + 1);
	agregar_a_paquete(paquete, trip, strlen(trip) + 1);

	enviar_paquete(paquete, socket);

	tripulante->tarea = recibir_tarea_de_RAM(socket);

	// free(patota);
	free(trip);
	eliminar_paquete(paquete);
	liberar_conexion(socket);
	pthread_mutex_unlock(&mutex_mostrar_por_consola);
}

//mensajes a IMONGOSTORE

/*
 ●  Se mueve de X|Y a X’|Y’
 ● Comienza ejecución de tarea X
 ● Se finaliza la tarea X
 ● Se corre en pánico hacia la ubicación del sabotaje
 ● Se resuelve el sabotaje
 */

// void enviar_info_para_bitacora_a_imongostore(t_tripulante *tripulante, int socket)
// {
// 	t_paquete *paquete = crear_paquete();
// 	paquete->codigo_operacion = RECIBIR_LA_UBICACION_DEL_TRIPULANTE;

// 	char *patota = string_itoa((tripulante->puntero_pcb));
// 	char *trip = string_itoa((tripulante->tid - 1) / 10);
// 	char *pos_x = string_itoa(tripulante->posicion_x);
// 	char *pos_y = string_itoa(tripulante->posicion_y);

// 	agregar_a_paquete(paquete, patota, strlen(patota) + 1);
// 	agregar_a_paquete(paquete, trip, strlen(trip) + 1);
// 	agregar_a_paquete(paquete, pos_x, strlen(pos_x) + 1);
// 	agregar_a_paquete(paquete, pos_y, strlen(pos_y) + 1);

// 	enviar_paquete(paquete, socket);
// 	free(patota);
// 	free(trip);
// 	free(pos_x);
// 	free(pos_y);

// 	eliminar_paquete(paquete);
// }