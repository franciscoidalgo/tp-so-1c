#include "imongostore.h"

int main(void){
t_log* logger = log_create("./cfg/imongostore.logger", "Imongostore", true, LOG_LEVEL_INFO);
log_info(logger, "Soy I Mongo Store! %s", mi_funcion_compartida());


int server_fd = iniciar_servidor(logger);

// void iterator(char* value)
// {
// 	 printf("%s\n", value);
// }
//log_info(logger,cliente_fd);
	t_list* lista;
	while(1)
	{	//int cliente_fd = esperar_cliente(server_fd,logger);
		struct sockaddr_in dir_cliente;
		int tam_direccion = sizeof(struct sockaddr_in);
		int cliente_fd = accept(server_fd, (void*) &dir_cliente, &tam_direccion);
		log_info(logger, "Se conecto un cliente!");
		int cod_op = recibir_operacion(cliente_fd);
		switch(cod_op)
		{
		case MENSAJE:
			recibir_mensaje(cliente_fd,logger);
			int size;
		char* buffer = recibir_buffer(&size, cliente_fd);
		log_info(logger, "Me llego el mensaje %s", buffer);
		free(buffer);
			break;
		case PAQUETE:
			lista = recibir_paquete(cliente_fd);
			printf("Me llegaron los siguientes valores:\n");
			// list_iterate(lista, (void*) iterator);
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


