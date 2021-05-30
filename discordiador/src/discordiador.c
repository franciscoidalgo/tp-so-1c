#include "discordiador.h"

int main(int argc, char** argv){

logger = log_create("./cfg/discordiador.log", "DISCORDIADOR", true, LOG_LEVEL_INFO);
log_info(logger, "Soy el discordiador! %s", mi_funcion_compartida());
t_config* config = leer_config();

READY =(t_tcb*) list_create();
NEW  =(t_tcb*) list_create();

char* ip = config_get_string_value(config,"IP");
char* puerto = config_get_string_value(config,"PUERTO");
char* valor;

valor=config_get_string_value(config,"CLAVE");

int conexion = crear_conexion(ip,puerto);
//////////////////////leer archivo y enviar tareas a MI-RAM
// FILE *f = fopen("/home/utnso/workspace/tp-2021-1c-Quinta-Recursada/discordiador/tareas/tareasPatota5.txt", "r");
// if (f==NULL)
// {
// 	puts("ERROR");
//    perror ("Error al abrir fichero.txt");
//    return -1;
// }
// char cadena[100]; /* Un array lo suficientemente grande como para guardar la línea más larga del fichero */
// 	char* palabra;//= string_itoa(a);//numero de la patota
// 	t_paquete* paquete = crear_paquete();
// strcpy(palabra,"11");
// while (fgets(cadena, 100, f) != NULL)
// {	
// 	log_info(logger,palabra);
// 	agregar_a_paquete(paquete,palabra,strlen(palabra)+1);
// 	strcpy(palabra,cadena);
// }
// enviar_paquete(paquete, conexion);
//fclose(f);
//char* aaa =  string_new();
 for (uint32_t i = 1 ; i <= atoi(argv[1]); i++){	
	log_info(logger,"INGRESANDO A LISTA NEW TRIPU %d DE PATOTA %s",i,argv[i+2]);
		int posx;
		int posy;
	if(argv[i+2]==NULL){
	posx = 0;
	posy = 0;
	}else{
	char** a = string_split(argv[i+2],",");
	posx = atoi(a[0]);
	posy = atoi(a[1]);
	}

	log_info(logger,"Tripulante %d posx:%d posy:%d patota:%d",i,posx,posy,atoi(argv[1]));
	//list_add(NEW, crear_tripulante(lim,  i+1,  i+2, i));
 }

//list_iterate(NEW, (void*) iterator);


exit(1);
//simular busqueda de tarea en simutaneo por tripulantes e ingresan a la lista de READY

while (list_size(NEW)!= 0)
{	
	t_tcb* tripulante;// = malloc(sizeof(t_tcb));
	tripulante = (t_tcb*) list_remove(NEW,0); // voy sacando de a uno los tripulantes

	//buscar_tarea_a_RAM((void*) tripulante);

	pthread_t hilo[tripulante->tid];
	if (0 == pthread_create(&hilo[tripulante->tid], NULL, (void *) &buscar_tarea_a_RAM,(void*) (tripulante)))
	{
		log_info(logger,"Tripulante %d de Patota %d busco la tarea en MI-RAM y se posicionó en READY",tripulante->tid,tripulante->puntero_pcb);
	}else{
		log_info(logger,"Tripulante %d no pudo ejecutar",tripulante->tid);
	}
	 
    pthread_join(hilo[tripulante->tid], NULL);

	//free(tripulante);
}

list_iterate(READY, (void*) iterator);
list_iterate(NEW, (void*) iterator);
// t_tcb* k = malloc(sizeof(t_tcb));
// k = list_get(NEW,1);
printf("Tamanio de lista %d \n", list_size(NEW));



//simular ejecucion de tare por cada tripulante con semaforo contador


// enviar CLAVE al servirdor
// enviar_mensaje(paquete_para_enviar,conexion);
terminar_programa(conexion, logger, config);
}

//////////////////////IMPLEMENTACION DE FUNCIONES//////////////////////////////////////////////
void iterator(t_tcb* t){
	 log_info(logger,"TID:%d POSX:%d POSY:%d ProximaInst:%d PCB:%d", t->tid,t->posicion_x,t->posicion_y,t->proxima_instruccion,t->puntero_pcb);
	 log_info(logger,"Estado: %c",t->estado);
 }


t_log* iniciar_logger(void)
{
	return log_create("discordiador.log","discordiador",true,LOG_LEVEL_INFO);
}

t_config* leer_config(void)
{
	return config_create("/home/utnso/workspace/tp-2021-1c-Quinta-Recursada/discordiador/cfg/discordiador.config");
}

void leer_consola(t_log* logger)
{
	char* leido;
	//El primero te lo dejo de yapa
	leido = readline(">");

	while (strcmp(leido,"") != 0)
	{
		log_info(logger,leido);
		free(leido);
		leido = readline(">");
	}

}


void terminar_programa(int conexion, t_log* logger, t_config* config)
{
	close(conexion);
	//Y por ultimo, para cerrar, hay que liberar lo que utilizamos (conexion, log y config) con las funciones de las commons y del TP mencionadas en el enunciado
log_destroy(logger);
}


void enviar_msj(char* mensaje, int socket_cliente)
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

	send(socket_cliente, magic, bytes, 0); //envia y espera la respuesta 

	free(magic);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}


t_tcb* crear_tripulante(uint32_t patota, uint32_t posx, uint32_t posy, uint32_t id){
	t_tcb* t = malloc(sizeof(t_tcb));

	t->posicion_x = posx;
	t->posicion_y = posy;
	t->proxima_instruccion = 0; //tarea,luego la busca en RAM
	t->tid=id;
	t->puntero_pcb = patota;
	t->estado='N';

	return t;
}

void buscar_tarea_a_RAM(void* tripu){

	t_tcb* t = (t_tcb*) tripu;
	t->proxima_instruccion = (uint32_t) rand() % 11;;
	t->estado='R';

	pthread_mutex_lock(&mutexSalirDeNEW);
	 	list_add(READY,t);		
    pthread_mutex_unlock(&mutexSalirDeNEW);  
}
