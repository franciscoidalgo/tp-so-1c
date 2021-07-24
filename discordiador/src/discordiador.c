#include "discordiador.h"

t_log* logger;
t_config* config;

int main(int argc, char ** argv){

// t_log* logger = log_create("./cfg/discordiador.log", "DISCORDIADOR", true, LOG_LEVEL_INFO);
logger = iniciar_logger("discordiador");
// log_info(logger, "Soy el discordiador! %s", mi_funcion_compartida());


/*---------------------------------------------------PARTE 2-------------------------------------------------------------*/
int conexion;
char* ip;
char* puerto;
// char* valor;


//Loggear "soy un log"
// log_info(logger,"soy un log");

config = leer_config("discordiador");

// log_info(logger,config_get_string_value(config,"CLAVE"));

	//asignar valor de config a la variable valor/
// valor=config_get_string_value(config,"CLAVE");
	
// log_info(logger,valor);


//Loggear valor de config

//leer_consola(logger);

	/*---------------------------------------------------PARTE 3-------------------------------------------------------------*/

	//antes de continuar, tenemos que asegurarnos que el servidor esté corriendo porque lo necesitaremos para lo que sigue.

	//crear conexion
	ip = config_get_string_value(config,"IP");
	puerto = config_get_string_value(config,"PUERTO");

	// log_info(logger,ip);
	// log_info(logger,puerto);


	//enviar CLAVE al servirdor
	// log_info(logger,valor);
	// enviar_mensaje(valor,conexion);

	// enviar_msj(valor,conexion,logger);

	// paquete(conexion);


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


	sleep(2);
	conexion = crear_conexion(ip,puerto);
	pthread_create(&otro_hilo,NULL, (void*) expulsar_tripulante_1_patota_2,conexion);
	pthread_join(otro_hilo,NULL);




	// sleep(2);
	// conexion = crear_conexion(ip,puerto);
	// pthread_create(&otro_hilo,NULL, (void*) expulsar_tripulante_2,conexion);
	// pthread_join(otro_hilo,NULL);

	sleep(2);
	conexion = crear_conexion(ip,puerto);
	pthread_create(&otro_hilo,NULL, (void*) envio_dump,conexion);
	pthread_join(otro_hilo,NULL);

	// sleep(2);
	// conexion = crear_conexion(ip,puerto);
	// pthread_create(&otro_hilo,NULL, (void*) envio_dump,conexion);
	// pthread_join(otro_hilo,NULL);


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

	//El primero te lo dejo de yapa
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