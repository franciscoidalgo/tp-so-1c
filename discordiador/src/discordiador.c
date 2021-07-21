#include "discordiador.h"
#include "planificadorFIFO.h"
#include "planificadorRR.h"

int main(int argc, char **argv)
{

	inicializar_variables();
	/*
enviar un mensaje a IMONGOSTORE para mantener una conexion activa (clavarme en un recv) y 
luego poder recibir la señal de sabotaje por ese "tunel" establecido
 */
	pthread_t hiloEscuchaSabotaje;
	pthread_create(&hiloEscuchaSabotaje, NULL, &atender_sabotaje, NULL);

	pthread_t hilo_recepcionista;
	while (1)
	{
		char *retorno_consola = readline(">");
		pthread_create(&hilo_recepcionista, NULL, &atender_accion_de_consola, retorno_consola);
	}
	pthread_join(hilo_recepcionista, NULL);

	log_destroy(logger);
	//terminar_variables_globales(conexion);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////IMPLEMENTACION DE FUNCIONES//////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void iterator(t_tcb *t)
{
	log_info(logger, "TID:%d POSX:%d POSY:%d PCB:%d ESTADO:%c", t->tid, t->posicion_x, t->posicion_y, t->puntero_pcb, t->estado);
}

void terminar_variables_globales(int socket)
{
	log_destroy(logger);
	liberar_conexion(socket);
	//config_destroy(config);
}

bool es_tripu_de_id(int id, t_tcb *tripulante)
{
	return tripulante->tid == id;
}
//inner_function
void expulsar_tripu(t_list *lista, int id_tripu)
{

	bool _el_tripulante_que_limpio(void *elemento)
	{
		return es_tripu_de_id(id_tripu, elemento);
	}

	t_tcb *tripulante = list_remove_by_condition(lista, _el_tripulante_que_limpio);
	free(tripulante);
}

t_tcb *remover_tripu(t_list *lista, int id_tripu)
{

	bool _el_tripulante_que_limpio(void *elemento)
	{
		return es_tripu_de_id(id_tripu, elemento);
	}

	return list_remove_by_condition(lista, _el_tripulante_que_limpio);
}

void inicializar_variables()
{
	ID_PATOTA = 0;
	logger = iniciar_logger("discordiador");
	log_info(logger, "Soy el discordiador! %s", mi_funcion_compartida());
	config = leer_config("discordiador");
	READY = list_create();
	BLOCKED_EMERGENCY = list_create();
	BLOCKED = list_create();
	EXIT = list_create();
	EXEC = list_create();
	NEW = list_create();
	primerIntento = true;
	pthread_mutex_init(&mutex, NULL);
	IP_RAM = config_get_string_value((t_config *)config, "IP");
	PUERTO_RAM = config_get_string_value((t_config *)config, "PUERTO");
	//sem_init(&semaforo_1,0,2);
	pthread_cond_init(&iniciarPlanificacion, NULL);
	NIVEL_MULTIPROCESAMIENTO = 2;
	//diccionario de acciones de consola
	dic_datos_consola = dictionary_create();
	dictionary_put(dic_datos_consola, "INICIAR_PATOTA", (void *)INICIAR_PATOTA);
	dictionary_put(dic_datos_consola, "INICIAR_PLANIFICACION", (void *)INICIAR_PLANIFICACION);
	dictionary_put(dic_datos_consola, "PAUSAR_PLANIFICACION", (void *)PAUSAR_PLANIFICACION);
	dictionary_put(dic_datos_consola, "EXPULSAR_TRIPULANTE", (void *)EXPULSAR_TRIPULANTE);
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

t_tcb *crear_tripulante(uint32_t patota, uint32_t posx, uint32_t posy, uint32_t tid)
{
	t_tcb *t = malloc(sizeof(t_tcb));

	t->posicion_x = posx;
	t->posicion_y = posy;
	//t->proxima_instruccion = 0; //tarea,luego la busca en RAM
	t->tid = tid;
	t->puntero_pcb = patota;
	t->estado = 'N';

	return t;
}

void buscar_tarea_a_RAM(t_tcb *t)
{
	pthread_mutex_lock(&mutex_mostrar_por_consola);
	char *patota_tripulante = string_new();
	string_append(&patota_tripulante, (string_itoa(t->puntero_pcb)));
	string_append(&patota_tripulante, "-");
	string_append(&patota_tripulante, (string_itoa(t->tid)));
	int socket_cliente = crear_conexion(IP_RAM, PUERTO_RAM);
	enviar_mensaje_and_codigo_op(patota_tripulante, MENSAJE, socket_cliente);
	t->tarea = recibir_tarea_de_RAM(socket_cliente);
	liberar_conexion(socket_cliente);
	free(patota_tripulante);
	pthread_mutex_unlock(&mutex_mostrar_por_consola);
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

	char cadena[50]; /* Un array lo suficientemente grande como para guardar la línea más larga del fichero */
	//strcpy(cadena,"");
	char *linea_file = malloc(50);
	t_paquete *paquete = crear_paquete();

	int id = ID_PATOTA;

	agregar_a_paquete(paquete, string_itoa(id), sizeof(id));
	agregar_a_paquete(paquete, linea_consola[1], strlen(linea_consola[1]) + 1);

	char *palabra_mas_guion;
	char *aux;

	while (fgets(cadena, 50, archivo) != NULL)
	{
		strcat(cadena, "\0");
		strcpy(linea_file, cadena);
		aux = string_duplicate(cadena);
		string_trim(&aux);
		strcat(aux, "-");
		palabra_mas_guion = malloc(strlen(aux) + 1);
		strcpy(palabra_mas_guion, aux);
		agregar_a_paquete(paquete, palabra_mas_guion, strlen(aux) + 1);
		strcpy(cadena, "");
		free(aux);
		free(palabra_mas_guion);
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
	free(linea_file);
	free(posiciones_linea);
	log_info(logger, "Enviando tareas a MI-RAM");
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
		log_info(logger, "POSICIONANDOME EN NEW TRIPU %d DE PATOTA %d", i, id_patota);
		posx = 0;
		posy = 0;
		if (linea_consola[i + 2] != NULL)
		{ //las posiciones comienzan desde el argumento 3 en adelante.
			posiciones = string_split(linea_consola[i + 2], "|");
			posx = atoi(posiciones[0]);
			posy = atoi(posiciones[1]);
		}

		//INICIARLIZAR TRIPULANTE
		t_tcb *tripulante = crear_tripulante(id_patota, posx, posy, i); // i --> concatenar con ID de patota

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

// t_tarea *deserealizar_tarea(t_buffer *buffer)
// {
// 	t_tarea *tarea = malloc(sizeof(t_tarea));

// 	void *stream = buffer->stream;
// 	// Deserializamos los campos que tenemos en el buffer
// 	memcpy(&(tarea->parametro), stream, sizeof(uint32_t));
// 	stream += sizeof(uint32_t);
// 	memcpy(&(tarea->posicion_x), stream, sizeof(uint32_t));
// 	stream += sizeof(uint32_t);
// 	memcpy(&(tarea->posicion_y), stream, sizeof(uint32_t));
// 	stream += sizeof(uint32_t);
// 	memcpy(&(tarea->tiempo), stream, sizeof(uint32_t));
// 	stream += sizeof(uint32_t);

// 	// Por último, para obtener el nombre, primero recibimos el tamaño y luego el texto en sí:
// 	memcpy(&(tarea->accion_length), stream, sizeof(uint32_t));
// 	stream += sizeof(uint32_t);
// 	tarea->accion = malloc(tarea->accion_length);
// 	memcpy(tarea->accion, stream, tarea->accion_length);

// 	return tarea;
// }

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

	char **tarea_completa_array = string_split(paquete->buffer->stream,";");
	char **tarea;

	t_tarea* tarea_recibida = malloc(sizeof(t_tarea));
	if(strcmp(paquete->buffer->stream,"NULL")==0){
		tarea_recibida->accion = "NULL";	
		return tarea_recibida;
		}

	if (string_contains(tarea_completa_array[0], " "))
	{
		tarea = string_split(tarea_completa_array[0], " ");
		tarea_recibida->accion = malloc(strlen(tarea[0]));
		strcpy(tarea_recibida->accion,tarea[0]);
		tarea_recibida->parametro = atoi(tarea[1]);
	}
	else
	{	tarea_recibida->accion = malloc(strlen(tarea_completa_array[0]));
		strcpy(tarea_recibida->accion,tarea_completa_array[0]);
		tarea_recibida->parametro = (-1);
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

void add_queue(int lista, t_tcb *tripulante)
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
		list_add(READY, tripulante);
		pthread_mutex_unlock(&mutex_lista_ready);
		break;
	case _BLOCKED_:;
		tripulante->estado = 'B';
		pthread_mutex_lock(&mutex_lista_blocked);
		list_add(BLOCKED, tripulante);
		pthread_mutex_unlock(&mutex_lista_blocked);
		break;
	case _BLOCKED_EMERGENCY_:;
		tripulante->estado = 'S';
		list_add(BLOCKED_EMERGENCY, tripulante);
		break;
	case _EXIT_:;
		tripulante->estado = 'F';
		pthread_mutex_lock(&mutex_lista_exit);
		list_add(EXIT, tripulante);
		pthread_mutex_unlock(&mutex_lista_exit);
		break;
	case _EXEC_:;
		tripulante->estado = 'E';
		pthread_mutex_lock(&mutex_lista_exec);
		list_add(EXEC, tripulante);
		pthread_mutex_unlock(&mutex_lista_exec);
		break;
	default:
		break;
	}
}

////////////////////////////////////////////////////atender_accion para hilo

void atender_accion_de_consola(char *linea_consola)
{

	char **array_parametros = string_split(linea_consola, " ");
	free(linea_consola);
	switch (get_diccionario_accion(array_parametros[0]))
	{
	case INICIAR_PATOTA:;
		ID_PATOTA = ID_PATOTA + 1;
		//SEND TAREAS A RAM
		int conexion = crear_conexion(IP_RAM, PUERTO_RAM);
		enviar_tareas_a_RAM(conexion, array_parametros);
		liberar_conexion(conexion);
		//SEND TAREAS A RAM
		recepcionar_patota(array_parametros);
		string_iterate_lines(array_parametros, iterator_lines_free);
		free(array_parametros);
		break;
	case INICIAR_PLANIFICACION:
		iniciar_planificacion();
		break;
	case LISTAR_TRIPULANTE:
		list_iterate(NEW, (void *)iterator);
		list_iterate(READY, (void *)iterator);
		list_iterate(BLOCKED, (void *)iterator);
		list_iterate(EXEC, (void *)iterator);
		list_iterate(EXIT, (void *)iterator);
		break;
	case EXPULSAR_TRIPULANTE:
		add_queue(_EXIT_, remover_tripu(NEW, atoi(array_parametros[1])));
		add_queue(_EXIT_, remover_tripu(READY, atoi(array_parametros[1])));
		add_queue(_EXIT_, remover_tripu(BLOCKED, atoi(array_parametros[1])));
		add_queue(_EXIT_, remover_tripu(EXEC, atoi(array_parametros[1])));
		//enviar_aviso_a_MI-RAM
		break;
	case OBTENER_BITACORA:
		//solicitar a IMONGOSTORE la bitacora del tripulante
		break;
	case PAUSAR_PLANIFICACION:
		pthread_mutex_lock(&mutex_planificacion);
		break;
	default:
		log_info(logger, "accion no disponible o cantidad de argumentos erronea, pifiaste en el tipeo hermano");
		break;
	}
}

void atender_sabotaje()
{
	// while (1)
	// {
	// 	log_info(logger, "Conectandome con IMONGOSTORE por situaciones de sabotaje");
	// 	int socket_conexion = crear_conexion(IP_RAM, PUERTO_RAM);
	// 	log_info(logger, "Socket %d id_sabotaje %d", socket_conexion, SABOTAJE);

	// 	int *f = SABOTAJE;

	// 	send(socket_conexion, &f, sizeof(int), 0);
	// 	int cod_op = 0;
	// 	recv(socket_conexion, &cod_op, sizeof(int), MSG_WAITALL);

	// 	list_remove(BLOCKED_EMERGENCY, 0);

	// 	if (cod_op == SABOTAJE)
	// 	{
	// 		log_info(logger, "Señal del SABOTAJE recibida");
	// 	}
	// }
}

iniciar_planificacion()
{

	//pthread_barrier_init(&barrera,NULL,tripulantes+1);

	sem_init(&sem_IO, 0, 1);
	sem_init(&sem_IO_queue, 0, 0);

	while (1)
	{
		pthread_mutex_lock(&mutex_planificacion);
		sem_init(&sem_exe, 0, NIVEL_MULTIPROCESAMIENTO);
		int tripulantes_en_new = list_size(NEW);
		pthread_t hilo[tripulantes_en_new];
		pthread_t planificador_ES;

		for (size_t i = 1; i <= tripulantes_en_new; i++)
		{
			t_tcb *tripulante = list_remove(NEW, 0);
			pthread_create(&hilo[i], NULL, &planificar_RR, tripulante);
		}
		///////PLANIFICA FIFO
		///////PLANIFICA RR
		///////PLANIFICA RR

		///////ENTRADA SALIDA
		pthread_create(&planificador_ES, NULL, &entrada_salida, NULL);
		pthread_detach(planificador_ES);
		///////ENTRADA SALIDA
		for (uint32_t j = 1; j <= tripulantes_en_new; j++)
		{
			pthread_detach(hilo[j]);
		}
	}
}