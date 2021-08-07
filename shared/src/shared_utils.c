#include "shared_utils.h"
#include <commons/string.h>

char* mi_funcion_compartida(){
    return "Hice uso de la shared!";
}

int iniciar_servidor(t_log* logger, t_config* config)
{
	int socket_servidor;

    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo((char*) config_get_string_value(config,"IP"), (char*) config_get_string_value(config,"PUERTO"), &hints, &servinfo);

    for (p=servinfo; p != NULL; p = p->ai_next)
    {
        if ((socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
            continue;

        if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1) {
            close(socket_servidor);
            continue;
        }
        break;
    }

	listen(socket_servidor, SOMAXCONN);

    freeaddrinfo(servinfo);

    // log_info(logger, "Listo para escuchar a mi cliente");

    return socket_servidor;
}

int esperar_cliente(int socket_servidor,t_log* logger)
{
	struct sockaddr_in dir_cliente;
	socklen_t * restrict tam_direccion = (socklen_t * restrict) sizeof(struct sockaddr_in);
	// socklen_t tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);
	/*
	accept --> funcion bloqueante, queda a la espera hasta aceptar cliente
	*/

	// log_info(logger, "Se conecto un cliente!");
	return socket_cliente;
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) != 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

void* recibir_buffer(int* direccion_size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, direccion_size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*direccion_size);
	recv(socket_cliente, buffer, *direccion_size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(int socket_cliente, t_log* logger,int* direccion_size)
{
	// int size;
	void* buffer = recibir_buffer(direccion_size, socket_cliente);
	log_info(logger, "Me llego el mensaje %s", (char*) buffer);
	free(buffer);
}

//podemos usar la lista de valores para poder hablar del for y de como recorrer la lista
void recibir_paquete(int socket_cliente, t_list* direccion_lista)
{
	int size;
	int desplazamiento = 0;
	void * buffer;
	// t_list* valores = list_create();
	int tamanio;

	char* valor;

	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(direccion_lista,(void*) valor);
		// free(valor);
	}
	free(buffer);
	// return valores;
}



void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * ptr_inicio_paquete = malloc(bytes);
	int desplazamiento = 0;

	memcpy(ptr_inicio_paquete + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(ptr_inicio_paquete + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(ptr_inicio_paquete + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	// desplazamiento+= paquete->buffer->size;

	return ptr_inicio_paquete;
}

void enviar_bitacora(t_bitacora* bitacora, int socket_cliente){
	char* msj = string_from_format("%lu-%lu-%s\n", bitacora->id_patota, bitacora->id_tripulante, bitacora->mensaje);
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = BITACORA;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(msj) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, msj, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}

char* obtener_bitacoras(uint32_t id_patota, uint32_t id_tripulante, int socket_cliente){
	int size;
	char* msj = string_from_format("%lu-%lu", id_patota, id_tripulante);
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = ENVIAR_BITACORA;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(msj) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, msj, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);

	recibir_operacion(socket_cliente);
	return recibir_buffer(&size, socket_cliente);

}


int crear_conexion(char *ip, char* puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1)
		printf("error");

	freeaddrinfo(server_info);

	return socket_cliente;
}

void enviar_mensaje(char* mensaje, int socket_cliente)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}


void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

// t_paquete* crear_super_paquete(void)
// {
// 	//me falta un malloc!
// 	t_paquete* paquete;

// 	//descomentar despues de arreglar
// 	// paquete->codigo_operacion = PAQUETE;
// 	//crear_buffer(paquete);
// 	return paquete;
// }

t_paquete* crear_paquete(int cod_operacion)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = cod_operacion;
	crear_buffer(paquete);
	return paquete;
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}

t_log* iniciar_logger(char* name)
{
	char* path = string_new();
	char* name_mayus = string_new();
	
	string_append(&path,"cfg/");
	string_append(&path,name);
	string_append(&path,".log");

	string_append(&name_mayus, name);
	string_to_upper(name_mayus);
	t_log* logger = log_create(path,name_mayus,true,LOG_LEVEL_INFO);
	
	free(path);
	free(name_mayus);

	return logger;
}

t_config* leer_config(char* name)
{
	char* path = string_new();
	string_append(&path, "cfg/");
	string_append(&path, name);
	string_append(&path, ".config");
	
	t_config* config = config_create(path);

	free(path);

	return config;
}

void terminar_programa(int conexion, t_log* logger, t_config* config)
{
	log_destroy(logger);
	liberar_conexion(conexion);
	config_destroy(config);
}

void validar_logger(t_log* logger)
{
	if(logger == NULL)
    {
        printf("No se pudo crear el logger\n");
		fflush(stdout);
    }
}

void validar_config(t_config* config)
{
	if(config == NULL)
    {
        printf("No se pudo crear el config\n");
		fflush(stdout);
    }
}
