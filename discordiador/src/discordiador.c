#include "discordiador.h"

int main(int argc, char** argv){

/////////////////////inicializo variables globales y locales//////////////////////////////////////////////////
logger = log_create("./cfg/discordiador.log", "DISCORDIADOR", true, LOG_LEVEL_INFO);
log_info(logger, "Soy el discordiador! %s", mi_funcion_compartida());
t_config* config = leer_config();
READY = list_create();
NEW  =list_create();
char* ip = config_get_string_value(config,"IP");
char* puerto = config_get_string_value(config,"PUERTO");
int conexion = crear_conexion(ip,puerto);

///////////////////////////////////////////////////leer archivo y enviar contenido a MI-RAM//////////////////////////////////////////
//enviar_tareas_a_RAM(conexion,argv[2],patota);
FILE *archivo = fopen(argv[2], "r");
if (archivo==NULL)
{
   puts("ERROR");
   perror ("Error al abrir fichero.txt");
   return -1;
}

char cadena[50]; /* Un array lo suficientemente grande como para guardar la línea más larga del fichero */
char* palabra = malloc(sizeof(char));//= string_itoa(a);//numero de la patota
t_paquete* paquete = crear_paquete();

agregar_a_paquete(paquete,argv[1],strlen(argv[1])+1);

while (fgets(cadena, 50, archivo) != NULL)
{	
	strcpy(palabra,cadena);
	//log_info(logger,"%s",palabra);
	agregar_a_paquete(paquete,palabra,strlen(palabra)+1);	
}
log_info(logger,"Enviando tareas a MI-RAM");
sleep(2);
enviar_paquete(paquete, conexion);
fclose(archivo);

 for (uint32_t i = 1 ; i <= atoi(argv[1]); i++){	
	log_info(logger,"INGRESANDO A LISTA NEW TRIPU %d DE PATOTA %d",i,atoi(argv[1]));
	int posx;
	int posy;
	char** posiciones;

	if(argv[i+2]==NULL){
	posx = 0;
	posy = 0;
	}else{
		posiciones = string_split(argv[i+2],"|");
		posx = atoi(posiciones[0]);
		posy = atoi(posiciones[1]);
	}

	list_add(NEW, crear_tripulante(atoi(argv[1]), posx,  posy, i));

	//string_iterate_lines(posiciones, free);
    //free(posiciones);
 }

list_iterate(NEW, (void*) iterator);

////////////////////////////Simular busqueda de tarea en simutaneo por tripulantes e ingresan a la lista de READY//////////
while (list_size(NEW)!= 0)
{	
	t_tcb* tripulante;// = malloc(sizeof(t_tcb));
	tripulante = (t_tcb*) list_remove(NEW,0); // voy sacando de a uno los tripulantes

	//buscar_tarea_a_RAM((void*) tripulante);
	pthread_t hilo[tripulante->tid];
	if (0 != pthread_create(&hilo[tripulante->tid], NULL, (void *) &buscar_tarea_a_RAM,(void*) (tripulante)))
	{
		log_info(logger,"Tripulante %d no pudo ejecutar",tripulante->tid);
	}
	 
    //pthread_join(hilo[tripulante->tid], NULL);

}

list_iterate(READY, (void*) iterator);

//Ejecucion de tarea por cada tripulante con semaforo contador


terminar_programa(conexion, logger, config);
}

////////////////////////////////////////IMPLEMENTACION DE FUNCIONES//////////////////////////////////////////////
void iterator(t_tcb* t){
	 log_info(logger,"TID:%d POSX:%d POSY:%d ProximaInst:%d PCB:%d ESTADO:%c", t->tid,t->posicion_x,t->posicion_y,t->proxima_instruccion,t->puntero_pcb,t->estado);
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
	log_info(logger,"Buscando tarea a MI-RAM soy tripulante %d de patota %d",t->tid,t->puntero_pcb);
	sleep(2);

	pthread_mutex_lock(&mutexSalirDeNEW);
	 	list_add(READY,t);		
    pthread_mutex_unlock(&mutexSalirDeNEW);  
}
