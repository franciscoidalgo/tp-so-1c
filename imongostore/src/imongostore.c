#include "imongostore.h"

int main(void){
t_log* logger = log_create("./cfg/imongostore.logger", "Imongostore", true, LOG_LEVEL_INFO);
log_info(logger, "Soy I Mongo Store! %s", mi_funcion_compartida());

t_config* config = leer_config("imongostore");
int server_fd = iniciar_servidor(logger,config);

// void iterator(char* value)
// {
// 	 printf("%s\n", value);
// }
//log_info(logger,cliente_fd);
	// t_list* lista;  //descomentar esto cuando se use el envio de paquete
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
			// lista = recibir_paquete(cliente_fd);
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


