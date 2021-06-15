#include "discordiador.h"

t_list * patotas;
typedef struct datosPatotas{
	int pid;
}t_patota;


int main(int argc, char ** argv){

// t_log* logger = log_create("./cfg/discordiador.log", "DISCORDIADOR", true, LOG_LEVEL_INFO);
t_log* logger = iniciar_logger("discordiador");
log_info(logger, "Soy el discordiador! %s", mi_funcion_compartida());


/*---------------------------------------------------PARTE 2-------------------------------------------------------------*/
int conexion;
char* ip;
char* puerto;
char* valor;


//Loggear "soy un log"
log_info(logger,"soy un log");

t_config* config = leer_config("discordiador");

log_info(logger,config_get_string_value(config,"CLAVE"));

	//asignar valor de config a la variable valor
valor=config_get_string_value(config,"CLAVE");
	
log_info(logger,valor);


//Loggear valor de config

//leer_consola(logger);

	/*---------------------------------------------------PARTE 3-------------------------------------------------------------*/

	//antes de continuar, tenemos que asegurarnos que el servidor esté corriendo porque lo necesitaremos para lo que sigue.

	//crear conexion
	ip = config_get_string_value(config,"IP");
	puerto = config_get_string_value(config,"PUERTO");

	log_info(logger,ip);
	log_info(logger,puerto);

	// conexion = crear_conexion(ip,puerto);
	conexion = crear_conexion(ip,puerto);

	//enviar CLAVE al servirdor
	log_info(logger,valor);
	enviar_mensaje(valor,conexion);

	// enviar_msj(valor,conexion,logger);

	// paquete(conexion);

	terminar_programa(conexion, logger, config);
}



// t_log* iniciar_logger(void)
// {
// 	return log_create("discordiador.log","discordiador",true,LOG_LEVEL_INFO);
// }

// t_config* leer_config(void)
// {
// 	return config_create("cfg/discordiador.config");
// }

void leer_consola(t_log* logger)
{
	char* leido;

	leido = readline(">");

	while (strcmp(leido,"") != 0)
	{
		log_info(logger,(char*)leido);
		free(leido);
		leido = readline(">");
	}
	free(leido);
}

// void paquete(int conexion)
// {
// 	//Ahora toca lo divertido!

// 	char* leido;
// 	t_paquete* paquete;


// }

// void terminar_programa(int conexion, t_log* logger, t_config* config)
// {
// 	//Y por ultimo, para cerrar, hay que liberar lo que utilizamos (conexion, log y config) con las funciones de las commons y del TP mencionadas en el enunciado
// 	log_destroy(logger);
// 	liberar_conexion(conexion);
// 	config_destroy(config);
// }

void enviar_msj(char* mensaje, int socket_cliente,t_log* logger)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	//void* a_enviar = serializar_paquete(paquete, bytes);
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	log_info(logger,mensaje);

	send(socket_cliente, magic, bytes, 0);

	free(magic);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}