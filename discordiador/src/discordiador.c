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
//levantar como sevidor de imongostore para escuchar cuando se dan los sabotajes

// initSem(semaforo,0,1);
// doWait(semaforo,0);

//int conexion = crear_conexion(IP,PUERTO);
int conexion;
while(1){
conexion = crear_conexion(IP,PUERTO);
	char* linea_consola = readline(">>");
	char** array_parametros = string_split(linea_consola," ");
	free(linea_consola);

int accion = dictionary_get(dic_datos_consola,array_parametros[0]);

switch(accion){
case INICIAR_PATOTA:
	enviar_tareas_a_RAM(conexion,(char*) array_parametros);
	recepcionar_patota(array_parametros);
	string_iterate_lines(array_parametros, iterator_lines_free);
	free(array_parametros);
	break;
case INICIAR_PLANIFICACION:
	busqueda_de_tareas_por_patota();
	break;
case LISTAR_TRIPULNATE:

	break;
case EXPULSAR_TRIPULANTE:

	break;
case OBTENER_BITACORA:

	break;
default:
	break;
}
}
list_iterate(NEW, (void*) iterator);

busqueda_de_tareas_por_patota();

list_iterate(READY, (void*) iterator);

terminar_variables_globales(conexion);
}
////////////////////////////////////////IMPLEMENTACION DE FUNCIONES//////////////////////////////////////////////
void iterator(t_tcb* t)
{
	 log_info(logger,"TID:%d POSX:%d POSY:%d PCB:%d ESTADO:%c", t->tid,t->posicion_x,t->posicion_y,t->puntero_pcb,t->estado);
}

void terminar_variables_globales(int socket){
	log_destroy(logger);
	liberar_conexion(socket);
	//config_destroy(config);
}

void inicializar_variables(){
logger = iniciar_logger("discordiador");
log_info(logger, "Soy el discordiador! %s", mi_funcion_compartida());
config = leer_config("discordiador");
READY = list_create();
NEW  =list_create();
 //Creamos un semaforo y damos permisos para compartirlo
    if((semaforo=semget(IPC_PRIVATE,1,IPC_CREAT | 0700))<0) {
        perror(NULL);
        error("Semaforo: semget");
        }


IP = config_get_string_value((t_config*) config,"IP");
PUERTO = config_get_string_value((t_config*) config,"PUERTO");

//diccionario de acciones de consola
dic_datos_consola = dictionary_create();
dictionary_put(dic_datos_consola,"INICIAR_PATOTA",INICIAR_PATOTA);
dictionary_put(dic_datos_consola,"INICIAR_PLANIFICACION",INICIAR_PLANIFICACION);
dictionary_put(dic_datos_consola,"PAUSAR_PLANIFICACION",PAUSAR_PLANIFICACION);
dictionary_put(dic_datos_consola,"EXPULSAR_TRIPULANTE",EXPULSAR_TRIPULANTE);
dictionary_put(dic_datos_consola,"LISTAR_TRIPULNATE",LISTAR_TRIPULNATE);
dictionary_put(dic_datos_consola,"OBTENER_BITACORA",OBTENER_BITACORA);
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
	//t->proxima_instruccion = 0; //tarea,luego la busca en RAM
	t->tid=id;
	t->puntero_pcb = patota;
	t->estado='N';

	return t;
}

void buscar_tarea_a_RAM(void* tripu){
	t_tcb* t = (t_tcb*) tripu;
	t->estado='R';
	log_info(logger,"Buscando tarea a MI-RAM soy tripulante %d de patota %d",t->tid,t->puntero_pcb);
	sleep(2);
	pthread_mutex_lock(&mutexSalirDeNEW);
	 	list_add(READY,t);		
    pthread_mutex_unlock(&mutexSalirDeNEW);  
}

void enviar_tareas_a_RAM(int conexion,char** linea_consola){

//"/home/utnso/workspace/tp-2021-1c-Quinta-Recursada/discordiador/tareas/tareasPatota5.txt"
char* path = string_new();// = malloc(strlen("/home/utnso/workspace/tp-2021-1c-Quinta-Recursada/discordiador/tareas/"));
string_append(&path,"/home/utnso/workspace/tp-2021-1c-Quinta-Recursada/discordiador/tareas/");
string_append(&path,linea_consola[2]);

FILE *archivo = fopen(path, "r");
if (archivo==NULL)
{
   puts("ERROR");
   perror ("Error al abrir fichero.txt");
   return -1;
}

char cadena[50]; /* Un array lo suficientemente grande como para guardar la línea más larga del fichero */
char* palabra = malloc(50);//= string_itoa(a);//numero de la patota
t_paquete* paquete = crear_paquete();

agregar_a_paquete(paquete,linea_consola[1],strlen(linea_consola[1])+1);

while(fgets(cadena, 50, archivo) != NULL)
{	
	strcpy(palabra,cadena);
	//log_info(logger,"%s",palabra);
	agregar_a_paquete(paquete,palabra,strlen(palabra)+1);	
}

	char* posiciones_linea =string_new();
for (int i = 3; linea_consola[i]!=NULL; i++){	
	string_append(&posiciones_linea,linea_consola[i]);
	if(linea_consola[i+1]!=NULL){
	string_append(&posiciones_linea,";");
	}
}

agregar_a_paquete(paquete,posiciones_linea,strlen(posiciones_linea)+1);
free(palabra);
free(posiciones_linea);
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

void iterator_lines_free(char* string)
{
    free(string);
}


///SEMAFOROS BINARIOS
void doSignal(int semid, int numSem) {
    struct sembuf sops; //Signal
    sops.sem_num = numSem;
    sops.sem_op = 1;
    sops.sem_flg = 0;

    if (semop(semid, &sops, 1) == -1) {
        perror(NULL);
        error("Error al hacer Signal");
    }
}

void doWait(int semid, int numSem) {
    struct sembuf sops;
    sops.sem_num = numSem; /* Sobre el primero, ... */
    sops.sem_op = -1; /* ... un wait (resto 1) */
    sops.sem_flg = 0;

    if (semop(semid, &sops, 1) == -1) {
        perror(NULL);
        error("Error al hacer el Wait");
    }
}

void initSem(int semid, int numSem, int valor) { //iniciar un semaforo
  
    if (semctl(semid, numSem, SETVAL, valor) < 0) {        
    perror(NULL);
        error("Error iniciando semaforo");
    }
}