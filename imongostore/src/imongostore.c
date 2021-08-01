#include "imongostore.h"

int main(void)
{
	logger = log_create("./cfg/imongostore.logger", "Imongostore", true, LOG_LEVEL_INFO);
	log_info(logger, "Soy I Mongo Store! %s", mi_funcion_compartida());

	int server_fd = iniciar_servidor(logger);

	a=0;
	b=0;
	c=0;
	tareas=0;
	TAREAS_GLOBAL = malloc(sizeof(t_tareas));
	TAREAS_GLOBAL->tareas = list_create();
	lista_tareas = list_create();

	while (1)
	{
		int cliente_fd = esperar_cliente(server_fd, logger);

		pthread_t hilo_accept;

		pthread_create(&hilo_accept, NULL, (void *)&atender_cliente, (void *)(cliente_fd));
	}

	log_destroy(logger);
	return EXIT_SUCCESS;
}

//////////////////////IMPLEMENTACION DE FUNCIONES//////////////////////////////////////////////

t_tarea *recibir_tarea(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void *buffer;
	t_list *valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while (desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento += sizeof(int);
		char *valor = malloc(tamanio);
		memcpy(valor, buffer + desplazamiento, tamanio);
		desplazamiento += tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	//return valores;
	return NULL;
}

char *accion;		 // Accion de la tarea
uint32_t parametro;	 // Numero relacionado a la tarea
uint32_t posicion_x; // Pos x
uint32_t posicion_y; // Pos y
uint32_t tiempo;	 // Tiempo en realizar la tarea

void enviar_tarea(t_tarea *tarea, int unSocket)
{

	t_buffer *buffer = malloc(sizeof(t_buffer));

	buffer->size = sizeof(uint32_t) * 5			// parametro,posx,posy,tiempo en realizar la tarea
				   + strlen(tarea->accion) + 1; // La longitud del string nombre. Le sumamos 1 para enviar tambien el caracter centinela '\0'. Esto se podría obviar, pero entonces deberíamos agregar el centinela en el receptor.

	void *stream = malloc(buffer->size);
	int offset = 0; // Desplazamiento

	memcpy(stream + offset, &tarea->parametro, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &tarea->posicion_x, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &tarea->posicion_y, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &tarea->tiempo, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	// Para el nombre primero mandamos el tamaño y luego el texto en sí:
	memcpy(stream + offset, &tarea->accion_length, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, tarea->accion, strlen(tarea->accion) + 1);
	// No tiene sentido seguir calculando el desplazamiento, ya ocupamos el buffer completo

	buffer->stream = stream;

	// Si usamos memoria dinámica para el nombre, y no la precisamos más, ya podemos liberarla:
	free(tarea->accion);

	///////////////Ahora podemos llenar el paquete con el bufer
	t_paquete *paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE; // Podemos usar una constante por operación
	paquete->buffer = buffer;			 // Nuestro buffer de antes.

	// Armamos el stream a enviar
	void *a_enviar = malloc(buffer->size + sizeof(uint32_t) + sizeof(uint32_t));
	int offset_buffer = 0;

	memcpy(a_enviar + offset_buffer, &(paquete->codigo_operacion), sizeof(uint32_t));
	offset_buffer += sizeof(uint32_t);
	memcpy(a_enviar + offset_buffer, &(paquete->buffer->size), sizeof(uint32_t));
	offset_buffer += sizeof(uint32_t);
	memcpy(a_enviar + offset_buffer, paquete->buffer->stream, paquete->buffer->size);

	// Por último enviamos
	send(unSocket, a_enviar, buffer->size + sizeof(uint32_t) + sizeof(uint32_t), 0);

	// No nos olvidamos de liberar la memoria que ya no usaremos
	free(a_enviar);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	//free(paquete);
}

void atender_cliente(int socket_cliente)
{
	int primer_linea = 0;
	void iterator(char *value)
	{
		if (primer_linea == 0)
		{
			TAREAS_GLOBAL->pid = atoi(value);
			primer_linea = primer_linea + 1;
		}

		if (primer_linea > 1)
		{
			if (!string_contains(value, "|") && string_contains(value, ";"))
			{
				list_add(TAREAS_GLOBAL->tareas, string_duplicate(value));
				//list_add(TAREAS_GLOBAL->tareas, string_substring_until(value,string_length(value)-1));
			}
		}

		if (primer_linea == 1)
		{
			primer_linea = primer_linea + 1;
		}

		log_info(logger, value);
	}

	// while (1)
	// {
	int cod_op = recibir_operacion(socket_cliente);
	switch (cod_op)
	{
	case ENVIAR_PROXIMA_TAREA:;
		pthread_mutex_lock(&mutex);
		lista = list_create();
		lista = recibir_paquete(socket_cliente);

		tareas = list_size(TAREAS_GLOBAL->tareas);
		int tripulante_id = (int)atoi(list_remove(lista,0));

		log_info(logger,"Se conecto el tripulante %d",tripulante_id);

		if(tripulante_id==1){
			if(tareas-1<a){
					enviar_mensaje("NULL", socket_cliente);

			} else{
			enviar_mensaje(list_get(TAREAS_GLOBAL->tareas, a), socket_cliente);
			a =a +1;}
		}
		
		if(tripulante_id==2){
			if(tareas-1<b){
					enviar_mensaje("NULL", socket_cliente);

			} else{
			enviar_mensaje(list_get(TAREAS_GLOBAL->tareas, b), socket_cliente);
			b =b +1;}
		}

		if(tripulante_id==3){
			if(tareas-1<c){
					enviar_mensaje("NULL", socket_cliente);

			} else{
			enviar_mensaje(list_get(TAREAS_GLOBAL->tareas, c), socket_cliente);
			c =c +1;}
		}
		

		liberar_conexion(socket_cliente);
		list_clean(lista);
		pthread_mutex_unlock(&mutex);
		break;
	case RECIBIR_LA_UBICACION_DEL_TRIPULANTE:;
		lista = recibir_paquete(socket_cliente);
		log_info(logger, "Me llegaron los siguientes valores");
		list_iterate(lista, (void *)iterator);
		list_clean(lista);
		break;
	case ACTUALIZAR_ESTADO:
		lista = recibir_paquete(socket_cliente);
		log_info(logger, "Me llegaron los siguientes valores");
		list_iterate(lista, (void *)iterator);
		list_clean(lista);
		break;
	case INICIAR_PATOTA:
		lista_tareas = recibir_paquete(socket_cliente);
		log_info(logger, "Me llegaron los siguientes valores");
		list_iterate(lista_tareas, (void *)iterator);
		break;
	case SABOTAJE:;
		// int direccion_size; 
		// recv((int)socket_cliente,direccion_size, sizeof(int), MSG_WAITALL);
		// void *buffer = malloc(direccion_size);
		// recv((int)socket_cliente, buffer,direccion_size, MSG_WAITALL);
		// log_info(logger, "Conectado con DISCORDIARDOR para situaciones de %s",buffer);

		//  sleep(15);

		// enviar_mensaje("ESTADO_DE_EMERGENCIA", socket_cliente);

		break;
	case -1:
		log_error(logger, "el cliente se desconecto. Terminando servidor");
		EXIT_FAILURE;
	default:
		log_warning(logger, "Operacion desconocida. No quieras meter la pata");
		break;
	}
	// }
}
