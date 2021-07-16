#include "../include/discordiador.h"
#include "planificadorFIFO.h"

int main(int argc, char **argv)
{

	inicializar_variables();
	/*
enviar un mensaje a IMONGOSTORE para mantener una conexion activa (clavarme en un recv) y 
luego poder recibir la señal de sabotaje por ese "tunel" establecido
 */
//int conexion = crear_conexion(IP, PUERTO);

pthread_mutex_init(&mutex_input_consola, NULL); 
pthread_t hilo_recepcionista;
	while (1)
	{
		char *linea_consola = readline("Ingresar_Comando>>");
		pthread_create(&hilo_recepcionista,NULL,atender_accion_de_consola,(void*) linea_consola);
	}

pthread_destroy(mutex_input_consola);

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

void iterator_buscar_tarea(t_tcb *tripu)
{
	pthread_t hilo[tripu->tid];
	if (0 != pthread_create(&hilo[tripu->tid], NULL, (void *)&buscar_tarea_a_RAM, (void *)(tripu)))
	{
		log_info(logger, "Tripulante %d no pudo ejecutar", tripu->tid);
	}
	//pthread_join(hilo[tripu->tid],NULL); //--> cada hilo espera a que el contiguo termine
}

void iterator_volver_join(t_tcb *tripu)
{ //EL SEND SERA BLOQUEANTE??
	pthread_t hilo[tripu->tid];
	pthread_detach(hilo[tripu->tid]);
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
	// int argumentos=0;
	// while(linea_consola[argumentos]!=NULL){
	// 	argumentos=argumentos+1;
	// }
	// log_info(logger,"argumentos ingresados: %d",argumentos);
	// int accion;
	// if(dictionary_has_key(dic_datos_consola,linea_consola[0])){
	// 	accion = dictionary_get(dic_datos_consola,linea_consola);
	// }
	// log_info(logger,"argumentos ingresados y accion: %d %d ",argumentos,accion);
	// bool cantidad_de_argumentos_OK = false;
	// if(accion==INICIAR_PATOTA && argumentos>2){
	// 	cantidad_de_argumentos_OK = true;}

	// if(accion==EXPULSAR_TRIPULANTE && argumentos==2){
	// 	cantidad_de_argumentos_OK = true;}

	// if((accion==INICIAR_PLANIFICACION || accion==PAUSAR_PLANIFICACION || accion==LISTAR_TRIPULANTE) && (argumentos==1)){
	// 	cantidad_de_argumentos_OK = true;}

	// if(cantidad_de_argumentos_OK){
	// 	return accion;}else{return -1;}
	// argumentos=0;
}

void enviar_msj(char *mensaje, int socket_cliente)
{
	t_paquete *paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
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

void buscar_tareas_desde_NEW()
{

	int size_new = list_size(NEW);

	for (size_t i = 1; i <= size_new; i++)
	{
		pthread_mutex_lock(&mutex_lista_new);
		t_tcb *tripu = remover_tripu(NEW, i);
		pthread_mutex_unlock(&mutex_lista_new);

		// pthread_condattr_init(&attr);
		// pthread_condattr_setclock(&attr,CLOCK_MONOTONIC);
		// pthread_cond_init(&cond, &attr);

		pthread_t hilo[tripu->tid];
		int err = pthread_create(&hilo[tripu->tid], NULL, (void *)&buscar_tarea_a_RAM, (void *)(tripu));
		if (err)
		{
			log_info(logger, "Tripulante %d no pudo ejecutar", tripu->tid);
		}

		// err = pthread_detach(hilo[tripu->tid]);
		// if(err){ log_info(logger,"Tripulante %d no pudo desprenderse",tripu->tid);}
	}

	size_new = 0;
}


////////////////////////////////////////////////////atender_accion para hilo

void atender_accion_de_consola(void* consola){

		char* linea_consola = (char*) consola;

		char **array_parametros = string_split(linea_consola, " ");
		free(linea_consola);
		switch (get_diccionario_accion(array_parametros[0]))
		{
		case INICIAR_PATOTA:;
			ID_PATOTA = ID_PATOTA + 1;
			int conexion = crear_conexion(IP, PUERTO);
			enviar_tareas_a_RAM(conexion, array_parametros);
			liberar_conexion(conexion);
			recepcionar_patota(array_parametros);
			string_iterate_lines(array_parametros, iterator_lines_free);
			free(array_parametros);
			list_iterate(NEW, iterator_buscar_tarea);
			//buscar_tareas_desde_NEW();
			break;
		case INICIAR_PLANIFICACION:
			planificar = true;
			planificar_FIFO();
			//list_iterate(READY,(void*) realizar_tarea_metodo_FIFO);
			break;
		case LISTAR_TRIPULANTE:
			list_iterate(NEW, (void *)iterator);
			list_iterate(READY, (void *)iterator);
			list_iterate(BLOCKED, (void *)iterator);
			break;
		case EXPULSAR_TRIPULANTE:
			expulsar_tripu(NEW, atoi(array_parametros[1]));
			expulsar_tripu(READY, atoi(array_parametros[1]));
			//enviar_aviso_a_MI-RAM
			break;
		case OBTENER_BITACORA:
			//solicitar a IMONGOSTORE la bitacora del tripulante
			break;
		case PAUSAR_PLANIFICACION:
			planificar = false;
			break;
		default:
			log_info(logger, "accion no disponible o cantidad de argumentos erronea, pifiaste en el tipeo hermano");
			break;
		}
}