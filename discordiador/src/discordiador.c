#include "../include/discordiador.h"
#include "planificadorFIFO.h"

t_log* logger;
t_config* config;

	inicializar_variables();
	/*
enviar un mensaje a IMONGOSTORE para mantener una conexion activa (clavarme en un recv) y 
luego poder recibir la señal de sabotaje por ese "tunel" establecido
 */
//int conexion = crear_conexion(IP, PUERTO);

// t_log* logger = log_create("./cfg/discordiador.log", "DISCORDIADOR", true, LOG_LEVEL_INFO);
logger = iniciar_logger("discordiador");
// log_info(logger, "Soy el discordiador! %s", mi_funcion_compartida());

pthread_destroy(mutex_input_consola);

/*---------------------------------------------------PARTE 2-------------------------------------------------------------*/
int conexion;
char* ip;
char* puerto;
// char* valor;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////IMPLEMENTACION DE FUNCIONES//////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void iterator(t_tcb *t)
{
	log_info(logger, "TID:%d POSX:%d POSY:%d PCB:%d ESTADO:%c", t->tid, t->posicion_x, t->posicion_y, t->puntero_pcb, t->estado);
}

//Loggear "soy un log"
// log_info(logger,"soy un log");

config = leer_config("discordiador");

// log_info(logger,config_get_string_value(config,"CLAVE"));

	//asignar valor de config a la variable valor/
// valor=config_get_string_value(config,"CLAVE");
	
// log_info(logger,valor);

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
	NEW = list_create();
	BLOCKED = list_create();
	pthread_mutex_init(&mutex, NULL);
	IP = config_get_string_value((t_config *)config, "IP");
	PUERTO = config_get_string_value((t_config *)config, "PUERTO");
	//sem_init(&semaforo_1,0,2);
	pthread_cond_init(&condicion_comunicacion_entre_tipos_de_ejecucion,NULL);
	//diccionario de acciones de consola
	dic_datos_consola = dictionary_create();
	dictionary_put(dic_datos_consola, "INICIAR_PATOTA", (void *)INICIAR_PATOTA);
	dictionary_put(dic_datos_consola, "INICIAR_PLANIFICACION", (void *)INICIAR_PLANIFICACION);
	dictionary_put(dic_datos_consola, "PAUSAR_PLANIFICACION", (void *)PAUSAR_PLANIFICACION);
	dictionary_put(dic_datos_consola, "EXPULSAR_TRIPULANTE", (void *)EXPULSAR_TRIPULANTE);
	dictionary_put(dic_datos_consola, "LISTAR_TRIPULANTE", (void *)LISTAR_TRIPULANTE);
	dictionary_put(dic_datos_consola, "OBTENER_BITACORA", (void *)OBTENER_BITACORA);
}

	// log_info(logger,ip);
	// log_info(logger,puerto);


	//enviar CLAVE al servirdor
	// log_info(logger,valor);
	// enviar_mensaje(valor,conexion);

	int bytes = paquete->buffer->size + 2 * sizeof(int);

	//void* a_enviar = serializar_paquete(paquete, bytes);
	void *magic = malloc(bytes);
	int desplazamiento = 0;


	conexion = crear_conexion(ip,puerto);
	pthread_t un_hilo,otro_hilo;
	pthread_create(&un_hilo,NULL, (void*) iniciar_patota,conexion);
	pthread_join(un_hilo,NULL);

	sleep(2);
	conexion = crear_conexion(ip,puerto);
	pthread_create(&un_hilo,NULL, (void*) iniciar_otra_patota,conexion);
	pthread_join(un_hilo,NULL);

	// sleep(2);
	// conexion = crear_conexion(ip,puerto);
	// pthread_create(&otro_hilo,NULL, (void*) envio_dump,conexion);
	// pthread_join(otro_hilo,NULL);


	// sleep(2);
	// conexion = crear_conexion(ip,puerto);
	// pthread_create(&un_hilo,NULL, (void*) iniciar_tercera_patota,conexion);
	// pthread_join(un_hilo,NULL);


	// sleep(2);
	// conexion = crear_conexion(ip,puerto);
	// pthread_create(&otro_hilo,NULL, (void*) enviar_actualizar_estado,conexion);
	// pthread_join(otro_hilo,NULL);


	// sleep(2);
	// conexion = crear_conexion(ip,puerto);
	// pthread_create(&otro_hilo,NULL, (void*) enviar_ubicacion_del_tripulante,conexion);
	// pthread_join(otro_hilo,NULL);

	// sleep(2);
	// conexion = crear_conexion(ip,puerto);
	// pthread_create(&otro_hilo,NULL, (void*) expulsar_tripulante_1,conexion);
	// pthread_join(otro_hilo,NULL);

	// sleep(2);
	// conexion = crear_conexion(ip,puerto);
	// pthread_create(&un_hilo,NULL, (void*) iniciar_tercera_patota,conexion);
	// pthread_join(un_hilo,NULL);

	// sleep(2);
	// conexion = crear_conexion(ip,puerto);
	// pthread_create(&otro_hilo,NULL, (void*) envio_dump,conexion);
	// pthread_join(otro_hilo,NULL);
// 



	// sleep(2);
	// conexion = crear_conexion(ip,puerto);
	// pthread_create(&otro_hilo,NULL, (void*) enviar_proxima_tarea,conexion);
	// pthread_join(otro_hilo,NULL);

	// sleep(2);
	// conexion = crear_conexion(ip,puerto);
	// pthread_create(&otro_hilo,NULL, (void*) enviar_proxima_tarea,conexion);
	// pthread_join(otro_hilo,NULL);

	// sleep(2);
	// conexion = crear_conexion(ip,puerto);
	// pthread_create(&otro_hilo,NULL, (void*) enviar_proxima_tarea,conexion);
	// pthread_join(otro_hilo,NULL);
	
	// sleep(2);
	// conexion = crear_conexion(ip,puerto);
	// pthread_create(&otro_hilo,NULL, (void*) enviar_proxima_tarea,conexion);
	// pthread_join(otro_hilo,NULL);

// 

	sleep(2);
	conexion = crear_conexion(ip,puerto);
	pthread_create(&otro_hilo,NULL, (void*) expulsar_tripulante_1,conexion);
	pthread_join(otro_hilo,NULL);


	sleep(2);
	conexion = crear_conexion(ip,puerto);
	pthread_create(&otro_hilo,NULL, (void*) expulsar_tripulante_3,conexion);
	pthread_join(otro_hilo,NULL);





	// sleep(2);
	// conexion = crear_conexion(ip,puerto);
	// pthread_create(&otro_hilo,NULL, (void*) expulsar_tripulante_2,conexion);
	// pthread_join(otro_hilo,NULL);

	sleep(2);
	conexion = crear_conexion(ip,puerto);
	pthread_create(&otro_hilo,NULL, (void*) envio_dump,conexion);
	pthread_join(otro_hilo,NULL);
	
	sleep(2);
	conexion = crear_conexion(ip,puerto);
	pthread_create(&otro_hilo,NULL, (void*) expulsar_tripulante_1_patota_2,conexion);
	pthread_join(otro_hilo,NULL);


	sleep(2);
	conexion = crear_conexion(ip,puerto);
	pthread_create(&otro_hilo,NULL, (void*) envio_dump,conexion);
	pthread_join(otro_hilo,NULL);


	// sleep(2);
	// conexion = crear_conexion(ip,puerto);
	// pthread_create(&otro_hilo,NULL, (void*) envio_compactar,conexion);
	// pthread_join(otro_hilo,NULL);

	// sleep(2);
	// conexion = crear_conexion(ip,puerto);
	// pthread_create(&otro_hilo,NULL, (void*) envio_dump,conexion);
	// pthread_join(otro_hilo,NULL);




	// char* resultado1, *resultado2;
	// log_info(logger,resultado1);
	// pthread_join(otro_hilo,NULL);
	// log_info(logger,resultado2);

	// iniciar_patota(conexion);
	// enviar_proxima_tarea(conexion);
	// free(resultado1);
	// free(resultado2);
	terminar_programa(conexion, logger, config);
}

t_tcb *crear_tripulante(uint32_t patota, uint32_t posx, uint32_t posy, uint32_t id)
{
	t_tcb *t = malloc(sizeof(t_tcb));

	t->posicion_x = posx;
	t->posicion_y = posy;
	//t->proxima_instruccion = 0; //tarea,luego la busca en RAM
	t->tid = id;
	t->puntero_pcb = patota;
	t->estado = 'N';

	return t;
}

void buscar_tarea_a_RAM(void *tripu)
{

	t_tcb *t = (t_tcb *)tripu;

	char *patota_tripulante = string_new();
	string_append(&patota_tripulante, (string_itoa(t->puntero_pcb)));
	string_append(&patota_tripulante, "-");
	string_append(&patota_tripulante, (string_itoa(t->tid)));
	//pthread_mutex_lock(&mutex);
	int socket_cliente = crear_conexion(IP, PUERTO);
	enviar_msj(patota_tripulante, socket_cliente);
	t->tarea = malloc(sizeof(t_tarea));
	t->tarea = recibir_tarea_de_RAM(socket_cliente);
	liberar_conexion(socket_cliente);
	free(patota_tripulante);
	//pthread_mutex_unlock(&mutex);
	t->estado = 'R';
	sleep(t->posicion_x + t->posicion_y);
	pthread_mutex_lock(&mutex_lista_ready);
	list_add(READY, remover_tripu(NEW, t->tid));
	pthread_mutex_unlock(&mutex_lista_ready);

	//pthread_mutex_lock(&mutex_mostrar_por_consola);
	//log_info(logger,"Tarea buscada %s tripulante %d de patota %d",(char*) t->tarea->accion,t->tid,t->puntero_pcb);
	//pthread_mutex_unlock(&mutex_mostrar_por_consola);
	//pthread_detach(pthread_self()); --> pareceria que no hace nada, libera recursos del la struct de hilo
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

	char cadena[50];			/* Un array lo suficientemente grande como para guardar la línea más larga del fichero */
	char *palabra = malloc(50); //= string_itoa(a);//numero de la patota
	t_paquete *paquete = crear_paquete();

	int id = ID_PATOTA;

	agregar_a_paquete(paquete, string_itoa(id), sizeof(id));
	agregar_a_paquete(paquete, linea_consola[1], strlen(linea_consola[1]) + 1);

	while (fgets(cadena, 50, archivo) != NULL)
	{
		strcpy(palabra, cadena);
		agregar_a_paquete(paquete, palabra, strlen(palabra) + 1);
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
	free(palabra);
	free(posiciones_linea);
	pthread_mutex_lock(&mutex_mostrar_por_consola);
	log_info(logger, "Enviando tareas a MI-RAM con socket: %d", conexion);
	pthread_mutex_unlock(&mutex_mostrar_por_consola);
	enviar_paquete(paquete, conexion);
	fclose(archivo);
}

void recepcionar_patota(char **linea_consola)
{
	int posx, posy;

	for (uint32_t i = 1; i <= atoi(linea_consola[1]); i++)
	{
		pthread_mutex_lock(&mutex_mostrar_por_consola);
		log_info(logger, "INGRESANDO A LISTA NEW TRIPU %d DE PATOTA %d", i, atoi(linea_consola[1]));
		pthread_mutex_unlock(&mutex_mostrar_por_consola);
		posx = 0;
		posy = 0;
		char **posiciones;

		if (linea_consola[i + 2] != NULL)
		{ //las posiciones comienzan desde el argumento 3 en adelante.
			posiciones = string_split(linea_consola[i + 2], "|");
			posx = atoi(posiciones[0]);
			posy = atoi(posiciones[1]);
		}

		list_add(NEW, crear_tripulante(atoi(linea_consola[1]), posx, posy, i));
	}
}

void busqueda_de_tareas_por_patota(t_tcb *tripulante)
{	//tendria que agregar como argumento el numero de patota y buscar en lista
	// int cantidad_de_tripulantes = list_size(NEW);

	// for (size_t i = 0; i <= cantidad_de_tripulantes; i++)
	// {
	pthread_t hilo[tripulante->tid];
	if (0 != pthread_create(&hilo[tripulante->tid], NULL, (void *)&buscar_tarea_a_RAM, (void *)(tripulante)))
	{
		log_info(logger, "Tripulante %d no pudo ejecutar", tripulante->tid);
	}

	// }
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

t_tarea *deserealizar_tarea(t_buffer *buffer)
{
	t_tarea *tarea = malloc(sizeof(t_tarea));

	void *stream = buffer->stream;
	// Deserializamos los campos que tenemos en el buffer
	memcpy(&(tarea->parametro), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	memcpy(&(tarea->posicion_x), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	memcpy(&(tarea->posicion_y), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	memcpy(&(tarea->tiempo), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	// Por último, para obtener el nombre, primero recibimos el tamaño y luego el texto en sí:
	memcpy(&(tarea->accion_length), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	tarea->accion = malloc(tarea->accion_length);
	memcpy(tarea->accion, stream, tarea->accion_length);

	return tarea;
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
	return deserealizar_tarea(paquete->buffer);
}

void realizar_tarea_metodo_FIFO(t_tcb *tripulante)
{

	//enviar inicio de la tarea a IMONGOSTORE junto con la tarea

	log_info(logger, "Tarea a realizar: %s", tripulante->tarea->accion);
	log_info(logger, "Me muevo de %d|%d a %d|%d ",
			 tripulante->posicion_x, tripulante->posicion_x, tripulante->tarea->posicion_x, tripulante->tarea->posicion_y);

	sleep(tripulante->tarea->tiempo);

	char *patota_tripulante = string_new();
	string_append(&patota_tripulante, "Termine mi tarea soy tripulante: ");
	string_append(&patota_tripulante, (string_itoa(tripulante->puntero_pcb)));
	string_append(&patota_tripulante, "-");
	string_append(&patota_tripulante, (string_itoa(tripulante->tid)));
	int socket_cliente = crear_conexion(IP, PUERTO);
	enviar_msj(patota_tripulante, socket_cliente);
	liberar_conexion(socket_cliente);
	free(patota_tripulante);
}

	free(magic);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void iniciar_patota(int conexion)
{
		// int cod_operacion = FINALIZACION; 
		int cod_operacion = INICIAR_PATOTA; 
	t_paquete* paquete = crear_paquete(cod_operacion);
	
	char* pid;
	pid = config_get_string_value(config,"PID");
	log_info(logger,pid);
	agregar_a_paquete(paquete,pid,strlen(pid)+1);
	
	char* cant_tripulantes = (char*) config_get_string_value(config,"CANT_TRIPULANTES");
	agregar_a_paquete(paquete,cant_tripulantes,strlen(cant_tripulantes)+1);

	// free(cant_tripulantes);

	// char* tarea = string_new();
	// char* parametro = string_new();
	char* tarea,*posiciones;
	// char* parametro;
	// char separador = (char) '-';
	
	tarea = config_get_string_value(config,"TAREA_1_COMPLETA");
	log_info(logger,tarea);
	agregar_a_paquete(paquete,tarea,strlen(tarea)+1);
	// free(tarea);

	tarea = config_get_string_value(config,"TAREA_2_COMPLETA");
	log_info(logger,tarea);
	agregar_a_paquete(paquete,tarea,strlen(tarea)+1);
	
	tarea = config_get_string_value(config,"TAREA_3_COMPLETA");
	log_info(logger,tarea);
	agregar_a_paquete(paquete,tarea,strlen(tarea)+1);

	posiciones = config_get_string_value(config,"POS");
	log_info(logger,posiciones);
	agregar_a_paquete(paquete,posiciones,strlen(posiciones)+1);


	enviar_paquete(paquete,conexion);
	eliminar_paquete(paquete);
}

void iniciar_otra_patota(int conexion)
{
		// int cod_operacion = FINALIZACION; 
		int cod_operacion = INICIAR_PATOTA; 
	t_paquete* paquete = crear_paquete(cod_operacion);
	
	char* pid;
	pid = config_get_string_value(config,"PID_OTRA");
	log_info(logger,pid);
	agregar_a_paquete(paquete,pid,strlen(pid)+1);
	
	char* cant_tripulantes = (char*) config_get_string_value(config,"CANT_TRIPULANTES_OTRA");
	agregar_a_paquete(paquete,cant_tripulantes,strlen(cant_tripulantes)+1);

	// free(cant_tripulantes);

	// char* tarea = string_new();
	// char* parametro = string_new();
	char* tarea,*posiciones;
	// char* parametro;
	// char separador = (char) '-';
	
	tarea = config_get_string_value(config,"TAREA_1_COMPLETA_OTRA");
	log_info(logger,tarea);
	agregar_a_paquete(paquete,tarea,strlen(tarea)+1);
	// free(tarea);

	tarea = config_get_string_value(config,"TAREA_2_COMPLETA_OTRA");
	log_info(logger,tarea);
	agregar_a_paquete(paquete,tarea,strlen(tarea)+1);
	
	tarea = config_get_string_value(config,"TAREA_3_COMPLETA_OTRA");
	log_info(logger,tarea);
	agregar_a_paquete(paquete,tarea,strlen(tarea)+1);

	posiciones = config_get_string_value(config,"POS_OTRA");
	log_info(logger,posiciones);
	agregar_a_paquete(paquete,posiciones,strlen(posiciones)+1);


	enviar_paquete(paquete,conexion);
	eliminar_paquete(paquete);
}

void iniciar_tercera_patota(int conexion)
{
		// int cod_operacion = FINALIZACION; 
		int cod_operacion = INICIAR_PATOTA; 
	t_paquete* paquete = crear_paquete(cod_operacion);
	
	char* pid;
	pid = config_get_string_value(config,"PID_TERCERA");
	log_info(logger,pid);
	agregar_a_paquete(paquete,pid,strlen(pid)+1);
	
	char* cant_tripulantes = (char*) config_get_string_value(config,"CANT_TRIPULANTES_TERCERA");
	agregar_a_paquete(paquete,cant_tripulantes,strlen(cant_tripulantes)+1);

	// free(cant_tripulantes);

	// char* tarea = string_new();
	// char* parametro = string_new();
	char* tarea,*posiciones;
	// char* parametro;
	// char separador = (char) '-';
	
	tarea = config_get_string_value(config,"TAREA_1_COMPLETA_TERCERA");
	log_info(logger,tarea);
	agregar_a_paquete(paquete,tarea,strlen(tarea)+1);
	// free(tarea);

	tarea = config_get_string_value(config,"TAREA_2_COMPLETA_TERCERA");
	log_info(logger,tarea);
	agregar_a_paquete(paquete,tarea,strlen(tarea)+1);
	
	tarea = config_get_string_value(config,"TAREA_3_COMPLETA_TERCERA");
	log_info(logger,tarea);
	agregar_a_paquete(paquete,tarea,strlen(tarea)+1);

	posiciones = config_get_string_value(config,"POS_TERCERA");
	log_info(logger,posiciones);
	agregar_a_paquete(paquete,posiciones,strlen(posiciones)+1);


	enviar_paquete(paquete,conexion);
	eliminar_paquete(paquete);
}


char* enviar_proxima_tarea(int conexion)
{
		// int cod_operacion = FINALIZACION; 
		int cod_operacion = ENVIAR_PROXIMA_TAREA; 
	t_paquete* paquete = crear_paquete(cod_operacion);
	
	char* pid, *tid;
	pid = config_get_string_value(config,"PID");
	log_info(logger,pid);
	agregar_a_paquete(paquete,pid,strlen(pid)+1);

	tid = config_get_string_value(config,"TID");
	log_info(logger,tid);
	agregar_a_paquete(paquete,tid,strlen(tid)+1);

	enviar_paquete(paquete,conexion);
	eliminar_paquete(paquete);



	// int* direccion_size;
	// recv(conexion, direccion_size, sizeof(int), MSG_WAITALL);
	// char* resultado = malloc(direccion_size);
	// recv(conexion, resultado, *direccion_size, MSG_WAITALL);
	// log_info(logger,"Te llego un mensajito %s",resultado);
	// return resultado;
	


	
	// t_list* respuesta = list_create();
	// recibir_paquete(conexion,respuesta);
	// char* resp = list_get(respuesta,0);
	// log_info(logger,"Te llego un mensajito %s",resp);
	// list_destroy(respuesta);
	return NULL;
}

char* enviar_actualizar_estado(int conexion)
{
		// int cod_operacion = FINALIZACION; 
		int cod_operacion = ACTUALIZAR_ESTADO; 
	t_paquete* paquete = crear_paquete(cod_operacion);
	
	char* pid, *tid;
	char* estado;
	pid = config_get_string_value(config,"PID");
	log_info(logger,pid);
	agregar_a_paquete(paquete,pid,strlen(pid)+1);

	tid = config_get_string_value(config,"TID");
	log_info(logger,tid);
	agregar_a_paquete(paquete,tid,strlen(tid)+1);

	estado = config_get_string_value(config,"ESTADO");
	// strcpy(est,estado);
	log_info(logger,estado);
	agregar_a_paquete(paquete,estado,strlen(estado)+1);

	enviar_paquete(paquete,conexion);
	eliminar_paquete(paquete);
	// int* direccion_size;
	// recv(conexion, direccion_size, sizeof(int), MSG_WAITALL);
	// char* resultado = malloc(direccion_size);
	// recv(conexion, resultado, *direccion_size, MSG_WAITALL);
	// return resultado;
	return NULL;
}

char* enviar_ubicacion_del_tripulante(int conexion)
{
		// int cod_operacion = FINALIZACION; 
		int cod_operacion = RECIBIR_LA_UBICACION_DEL_TRIPULANTE; 
	t_paquete* paquete = crear_paquete(cod_operacion);
	
	char* pid, *tid;
	char* estado;
	pid = config_get_string_value(config,"PID");
	log_info(logger,pid);
	agregar_a_paquete(paquete,pid,strlen(pid)+1);

	tid = config_get_string_value(config,"TID");
	log_info(logger,tid);
	agregar_a_paquete(paquete,tid,strlen(tid)+1);

	estado = config_get_string_value(config,"NUEVA_POS_X");
	log_info(logger,estado);
	agregar_a_paquete(paquete,estado,strlen(estado)+1);

	estado = config_get_string_value(config,"NUEVA_POS_Y");
	log_info(logger,estado);
	agregar_a_paquete(paquete,estado,strlen(estado)+1);

	enviar_paquete(paquete,conexion);
	eliminar_paquete(paquete);
	// int* direccion_size;
	// recv(conexion, direccion_size, sizeof(int), MSG_WAITALL);
	// char* resultado = malloc(direccion_size);
	// recv(conexion, resultado, *direccion_size, MSG_WAITALL);
	// return resultado;
	return NULL;
}

char* expulsar_tripulante_1(int conexion)
{
		// int cod_operacion = FINALIZACION; 
		int cod_operacion = EXPULSAR_TRIPULANTE; 
	t_paquete* paquete = crear_paquete(cod_operacion);
	
	char* pid, *tid;
	char* estado;
	pid = config_get_string_value(config,"PID");
	log_info(logger,pid);
	agregar_a_paquete(paquete,pid,strlen(pid)+1);

	tid = config_get_string_value(config,"TID");
	log_info(logger,tid);
	agregar_a_paquete(paquete,tid,strlen(tid)+1);

	enviar_paquete(paquete,conexion);
	eliminar_paquete(paquete);
	return NULL;
}

char* expulsar_tripulante_2(int conexion)
{
		// int cod_operacion = FINALIZACION; 
		int cod_operacion = EXPULSAR_TRIPULANTE; 
	t_paquete* paquete = crear_paquete(cod_operacion);
	
	char* pid, *tid;
	char* estado;
	pid = config_get_string_value(config,"PID");
	log_info(logger,pid);
	agregar_a_paquete(paquete,pid,strlen(pid)+1);

	tid = config_get_string_value(config,"TID_EXP_2");
	log_info(logger,tid);
	agregar_a_paquete(paquete,tid,strlen(tid)+1);

	enviar_paquete(paquete,conexion);
	eliminar_paquete(paquete);
	return NULL;
}

char* expulsar_tripulante_3(int conexion)
{
		// int cod_operacion = FINALIZACION; 
		int cod_operacion = EXPULSAR_TRIPULANTE; 
	t_paquete* paquete = crear_paquete(cod_operacion);
	
	char* pid, *tid;
	char* estado;
	pid = config_get_string_value(config,"PID_EXP");
	log_info(logger,pid);
	agregar_a_paquete(paquete,pid,strlen(pid)+1);

	tid = config_get_string_value(config,"TID_EXP_3");
	log_info(logger,tid);
	agregar_a_paquete(paquete,tid,strlen(tid)+1);

	enviar_paquete(paquete,conexion);
	eliminar_paquete(paquete);
	return NULL;
}

char* expulsar_tripulante_5(int conexion)
{
		// int cod_operacion = FINALIZACION; 
		int cod_operacion = EXPULSAR_TRIPULANTE; 
	t_paquete* paquete = crear_paquete(cod_operacion);
	
	char* pid, *tid;
	char* estado;
	pid = config_get_string_value(config,"PID_EXP");
	log_info(logger,pid);
	agregar_a_paquete(paquete,pid,strlen(pid)+1);

	tid = config_get_string_value(config,"TID_EXP_5");
	log_info(logger,tid);
	agregar_a_paquete(paquete,tid,strlen(tid)+1);

	enviar_paquete(paquete,conexion);
	eliminar_paquete(paquete);
	return NULL;
}

char* expulsar_tripulante_1_patota_2(int conexion)
{
		// int cod_operacion = FINALIZACION; 
		int cod_operacion = EXPULSAR_TRIPULANTE; 
	t_paquete* paquete = crear_paquete(cod_operacion);
	
	char* pid, *tid;
	char* estado;
	pid = config_get_string_value(config,"PID_OTRA");
	log_info(logger,pid);
	agregar_a_paquete(paquete,pid,strlen(pid)+1);

	tid = config_get_string_value(config,"TID_EXP_1");
	log_info(logger,tid);
	agregar_a_paquete(paquete,tid,strlen(tid)+1);

	enviar_paquete(paquete,conexion);
	eliminar_paquete(paquete);
	return NULL;
}

char* expulsar_tripulante_3_patota_2(int conexion)
{
		// int cod_operacion = FINALIZACION; 
		int cod_operacion = EXPULSAR_TRIPULANTE; 
	t_paquete* paquete = crear_paquete(cod_operacion);
	
	char* pid, *tid;
	char* estado;
	pid = config_get_string_value(config,"PID_OTRA");
	log_info(logger,pid);
	agregar_a_paquete(paquete,pid,strlen(pid)+1);

	tid = config_get_string_value(config,"TID_EXP_3");
	log_info(logger,tid);
	agregar_a_paquete(paquete,tid,strlen(tid)+1);

	enviar_paquete(paquete,conexion);
	eliminar_paquete(paquete);
	return NULL;
}


char* expulsar_tripulante_5_patota_2(int conexion)
{
		// int cod_operacion = FINALIZACION; 
		int cod_operacion = EXPULSAR_TRIPULANTE; 
	t_paquete* paquete = crear_paquete(cod_operacion);
	
	char* pid, *tid;
	char* estado;
	pid = config_get_string_value(config,"PID_OTRA");
	log_info(logger,pid);
	agregar_a_paquete(paquete,pid,strlen(pid)+1);

	tid = config_get_string_value(config,"TID_EXP_5");
	log_info(logger,tid);
	agregar_a_paquete(paquete,tid,strlen(tid)+1);

	enviar_paquete(paquete,conexion);
	eliminar_paquete(paquete);
	return NULL;
}

char* envio_compactar(int conexion)
{
		int cod_operacion = COMPACTAR; 
	t_paquete* paquete = crear_paquete(cod_operacion);
	
	char* pid, *tid;
	char* estado;
	pid = config_get_string_value(config,"PID_EXP");
	log_info(logger,pid);
	agregar_a_paquete(paquete,pid,strlen(pid)+1);

	tid = config_get_string_value(config,"TID_EXP_1");
	log_info(logger,tid);
	agregar_a_paquete(paquete,tid,strlen(tid)+1);

	enviar_paquete(paquete,conexion);
	eliminar_paquete(paquete);
	return NULL;
}

char* envio_dump(int conexion)
{
		int cod_operacion = DUMP; 
	t_paquete* paquete = crear_paquete(cod_operacion);
	
	char* pid, *tid;
	char* estado;
	pid = config_get_string_value(config,"PID_EXP");
	log_info(logger,pid);
	agregar_a_paquete(paquete,pid,strlen(pid)+1);

	tid = config_get_string_value(config,"TID_EXP_1");
	log_info(logger,tid);
	agregar_a_paquete(paquete,tid,strlen(tid)+1);

	enviar_paquete(paquete,conexion);
	eliminar_paquete(paquete);
	return NULL;
}