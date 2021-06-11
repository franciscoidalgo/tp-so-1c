#include "discordiador.h"

int main(int argc, char** argv){
//harcodeo para probar valgrind
argv[1]="4";
argv[2]="4";
argv[3]="1|3";
argv[4]="4|3";
argv[5]="10|3";
argv[6]=NULL;

inicializar_variables();
int conexion = crear_conexion(IP,PUERTO);

enviar_tareas_a_RAM(conexion,(char**) argv);

recepcionar_patota((char**) argv);

list_iterate(NEW, (void*) iterator);

busqueda_de_tareas_por_patota();

list_iterate(READY, (void*) iterator);

terminar_variables_globales(conexion);
}

////////////////////////////////////////IMPLEMENTACION DE FUNCIONES//////////////////////////////////////////////
void iterator(t_tcb* t)
{
	 log_info(logger,"TID:%d POSX:%d POSY:%d ProximaInst:%d PCB:%d ESTADO:%c", t->tid,t->posicion_x,t->posicion_y,t->proxima_instruccion,t->puntero_pcb,t->estado);
}

void terminar_variables_globales(int socket){
	log_destroy(logger);
	liberar_conexion(socket);
	//config_destroy(config);
}


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

void inicializar_variables(){
logger = iniciar_logger("discordiador");
log_info(logger, "Soy el discordiador! %s", mi_funcion_compartida());
config = leer_config("discordiador");
READY = list_create();
NEW  =list_create();
IP = config_get_string_value((t_config*) config,"IP");
PUERTO = config_get_string_value((t_config*) config,"PUERTO");
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
	t->proxima_instruccion = (uint32_t) rand() % 11;
	t->estado='R';
	log_info(logger,"Buscando tarea a MI-RAM soy tripulante %d de patota %d",t->tid,t->puntero_pcb);
	sleep(2);

	pthread_mutex_lock(&mutexSalirDeNEW);
	 	list_add(READY,t);		
    pthread_mutex_unlock(&mutexSalirDeNEW);  
}

void enviar_tareas_a_RAM(int conexion,char** argv){
FILE *archivo = fopen("/home/utnso/workspace/tp-2021-1c-Quinta-Recursada/discordiador/tareas/tareasPatota5.txt", "r");
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
}


void recepcionar_patota(char** a){
 int posx, posy;
 for (uint32_t i = 1 ; i <= atoi(a[1]); i++){	
	log_info(logger,"INGRESANDO A LISTA NEW TRIPU %d DE PATOTA %d",i,atoi(a[1]));
	posx = 0;
	posy = 0;
	char** posiciones;

	if(a[i+2]!=NULL){
		posiciones = string_split(a[i+2],"|");
		posx = atoi(posiciones[0]);
		posy = atoi(posiciones[1]);
	}

	list_add(NEW, crear_tripulante(atoi(a[1]), posx,  posy, i));

	// string_iterate_lines(posiciones, free);
    // free(posiciones);
 }
}

void busqueda_de_tareas_por_patota(){//tendria que agregar como argumento el numero de patota y buscar en lista
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
	 
    pthread_join(hilo[tripulante->tid], NULL);
}
}