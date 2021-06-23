#include "imongostore.h"

int main(void){
t_log* logger = log_create("./cfg/imongostore.logger", "Imongostore", true, LOG_LEVEL_INFO);
log_info(logger, "Soy I Mongo Store! %s", mi_funcion_compartida());


int server_fd = iniciar_servidor(logger);

int contador = 0;
TAREAS_GLOBAL = malloc(sizeof(t_tareas));
TAREAS_GLOBAL->tareas = malloc(sizeof(t_list));

t_list* list_aux = list_create();
void iterator(char* value)
{
	if(contador==0){
		TAREAS_GLOBAL->pid = atoi(value);
		contador = contador + 1;
	}else{
		list_add(TAREAS_GLOBAL->tareas,value); 
	}
	 log_info(logger, value);
}

	t_list* lista;
	while(1)
	{	
		int cliente_fd = esperar_cliente(server_fd,logger);
		int cod_op = recibir_operacion(cliente_fd);
		
		switch(cod_op)
		{
		case MENSAJE: ;
			int size;
			recibir_mensaje(cliente_fd,logger,&size);
			//harcodeo una tarea
			t_tarea* tarea_a_enviar = malloc(sizeof(t_tarea));
			char* accion = "SALTAR";
			tarea_a_enviar->accion =  malloc(strlen(accion));
			strcpy(tarea_a_enviar->accion,accion);
			tarea_a_enviar->accion_length = strlen(tarea_a_enviar->accion)+1;
			tarea_a_enviar->parametro = 5;
			tarea_a_enviar->posicion_x = 12;
			tarea_a_enviar->posicion_y = 22;
			tarea_a_enviar->tiempo = 10;
			enviar_tarea(tarea_a_enviar,cliente_fd);
			break;
		case PAQUETE:
			lista = recibir_paquete(cliente_fd);
			log_info(logger,"Me llegaron los siguientes valores del socket: %d",cliente_fd);
			list_iterate(lista, (void*) iterator);
			break;
		case -1:
			log_error(logger, "el cliente se desconecto. Terminando servidor");
			return EXIT_FAILURE;
		default:
			log_warning(logger, "Operacion desconocida. No quieras meter la pata");
			break;
		}
		
	}
	return EXIT_SUCCESS;



log_destroy(logger);
}

//////////////////////IMPLEMENTACION DE FUNCIONES//////////////////////////////////////////////


t_tarea* recibir_tarea(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	t_tarea* tarea = malloc(sizeof(t_tarea));

	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
	return NULL;
}

	char* accion;			// Accion de la tarea
	uint32_t parametro;		// Numero relacionado a la tarea
	uint32_t posicion_x;	// Pos x
	uint32_t posicion_y;	// Pos y
	uint32_t tiempo;		// Tiempo en realizar la tarea

void enviar_tarea(t_tarea* tarea,int unSocket){

t_buffer* buffer = malloc(sizeof(t_buffer));

buffer->size = sizeof(uint32_t) * 5 // parametro,posx,posy,tiempo en realizar la tarea
             + strlen(tarea->accion)+ 1; // La longitud del string nombre. Le sumamos 1 para enviar tambien el caracter centinela '\0'. Esto se podría obviar, pero entonces deberíamos agregar el centinela en el receptor.

void* stream = malloc(buffer->size);
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
t_paquete* paquete = malloc(sizeof(t_paquete));

paquete->codigo_operacion = MENSAJE; // Podemos usar una constante por operación
paquete->buffer = buffer; // Nuestro buffer de antes.

// Armamos el stream a enviar
void* a_enviar = malloc(buffer->size + sizeof(uint32_t) + sizeof(uint32_t));
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


