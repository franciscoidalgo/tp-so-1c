#include "imongostore.h"

int main(void){
t_log* logger = log_create("./cfg/imongostore.logger", "Imongostore", true, LOG_LEVEL_INFO);
log_info(logger, "Soy I Mongo Store! %s", mi_funcion_compartida());


int server_fd = iniciar_servidor(logger);

int contador = 0;
TAREAS_GLOBAL = malloc(sizeof(t_tareas));
TAREAS_GLOBAL->tareas_tripu = malloc(sizeof(t_list));

t_list* list_aux = list_create();
void iterator(char* value)
{
	if(contador==0){
		TAREAS_GLOBAL->pid = atoi(value);
		contador = contador + 1;
	}else{
		list_add(TAREAS_GLOBAL->tareas_tripu,value); 
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
            // char* buffer;
			recibir_mensaje(cliente_fd,logger,&size);
            // void* buffer = recibir_buffer(&size, cliente_fd);
            // log_info(logger, "Me llego el mensaje %p", buffer);
            // free(buffer);
			break;
		case PAQUETE:
			lista = recibir_paquete(cliente_fd);
			log_info(logger,"Me llegaron los siguientes valores:");
			list_iterate(lista, (void*) iterator);

			log_info(logger, string_itoa(TAREAS_GLOBAL->pid));
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


t_tareas* recibir_tareas(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	t_tareas* tareas = malloc(sizeof(t_tareas));

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


