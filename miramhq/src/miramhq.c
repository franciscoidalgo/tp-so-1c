#include "miramhq.h"

typedef struct {
    		uint32_t pid;
    		t_list* lista_de_marcos;
		} t_proceso_paginado;


typedef struct{
			uint32_t pid;		// Id de la patota
			uint32_t tareas;	// Indica el COMIENZO de la lista
		} t_pcb;


typedef struct{
			uint32_t tid;			// Id del tripulante
			char estado;			// Estado del tripulante (New/Ready/Exec/Blocked)
			uint32_t posicion_x;	// Pos x
			uint32_t posicion_y;	// Pos y
			uint32_t proxima_instruccion;	// instruccion que el tripulante debera hacer
			uint32_t puntero_pcb;	// quien es mi patota?
		} t_tcb;

typedef struct{
			t_pcb pcb;			
			t_list* lista_de_tcb; 		
			char* lista_de_tareas;
		} estructura_administrativa_paginacion;

typedef struct{
			int tamanio_data;			
			void* data_empaquetada; 		
		} dto_memoria;

void* MEMORIA;
uint32_t* MARCOS_DISPONIBLES;
t_list* tlb;
int TAMANIO_PAGINAS = 2;
int TAMANIO_MEMORIA = 100;
int CANTIDAD_MARCOS;
	


int obtener_tamanio_array_de_marcos (){
	if (TAMANIO_MEMORIA >= TAMANIO_PAGINAS) {
		//printf("La cantidad de marcos disponibles es de %d marcos\n", TAMANIO_MEMORIA / TAMANIO_PAGINAS);
		//printf("La cantidad de memoria que quedara inutilizable sera de %d bytes\n", TAMANIO_MEMORIA % TAMANIO_PAGINAS);
		return TAMANIO_MEMORIA/TAMANIO_PAGINAS;
	} else {
		printf("Hubo un error en la asignacion de MARCOS_DISPONIBLES");
		return 10;
	}
}

dto_memoria empaquetar_data_paginacion (estructura_administrativa_paginacion* data_a_empaquetar){
	dto_memoria dto_aux;

	int aux_tamanio = 0;
	int cantidad_de_tripulantes = list_size(data_a_empaquetar->lista_de_tcb);
	int largo_lista_de_tareas = strlen(data_a_empaquetar->lista_de_tareas) + 1;
	aux_tamanio += 8;
	aux_tamanio += cantidad_de_tripulantes * 21;
	data_a_empaquetar->pcb.tareas = aux_tamanio;
	aux_tamanio += largo_lista_de_tareas;
	
	dto_aux.tamanio_data = aux_tamanio;
	
	void* memaux;
	memaux = (void*) malloc (aux_tamanio);
	memcpy(memaux, &data_a_empaquetar->pcb, 8);
	for (int i = 0; i < cantidad_de_tripulantes; i++ ) {
		 t_tcb* aux_tcb = list_get(data_a_empaquetar->lista_de_tcb, i);
		 memcpy(memaux + 8 + 21*i, aux_tcb, 21);
	}
	memcpy(memaux + 8 + cantidad_de_tripulantes * 21, &data_a_empaquetar->lista_de_tareas, largo_lista_de_tareas);
	dto_aux.data_empaquetada = memaux;

	return dto_aux;
}

void modificar_tlb (uint32_t id_proceso, t_list* marcos_utilizados) {
	
	bool find(void* element){
		t_proceso_paginado* aux = element;
		return aux->pid == id_proceso;
	}

		list_remove_and_destroy_by_condition(tlb, find, free);
		t_proceso_paginado* dto = malloc(sizeof(t_proceso_paginado));
		dto->pid = id_proceso;
		dto->lista_de_marcos = marcos_utilizados;
		list_add(tlb, dto);
}

int obtener_marcos_vacios() {
	int marcos_vacios = 0;
	
	for(int i=0; i<CANTIDAD_MARCOS; i++) {
		if(MARCOS_DISPONIBLES[i]==0) {
			marcos_vacios++;
		}
	} 
	return marcos_vacios;
}

int paginas_que_ocupa(int cantidad_bytes){
	int aux = cantidad_bytes / TAMANIO_PAGINAS;
	if (cantidad_bytes % TAMANIO_PAGINAS != 0) {
		aux ++;
	}
	return aux;
}

void setear_memoria(t_list* lista_a_reservar, void* data_empaquetada){
	int aux = 0;
	void setear_una_pagina(int numero_marco){
		memcpy(MEMORIA + (numero_marco * TAMANIO_PAGINAS), data_empaquetada + aux, TAMANIO_PAGINAS);
		aux += TAMANIO_PAGINAS;
	}
	list_iterate(lista_a_reservar, setear_una_pagina);
}


t_list* obtener_marcos_a_reservar(int cantidad_solicitada){
	t_list* aux_list = list_create();
	int contador_array = 0;
	for(int i=0; i<cantidad_solicitada; i++) {
		if(MARCOS_DISPONIBLES[contador_array]==0) {
			list_add(aux_list, contador_array);
		} else {
			i--;
		}
		contador_array++;
	} 
	return aux_list;
}

void setear_marcos_usados(t_list* lista_a_reservar){
	void setear_marco_como_usado(int numero_marco){
		MARCOS_DISPONIBLES[numero_marco] = 1;
	}
	list_iterate(lista_a_reservar, setear_marco_como_usado);
}

void paginar (estructura_administrativa_paginacion* dato_a_paginar){
	dto_memoria dato_empaquetado = empaquetar_data_paginacion(dato_a_paginar);
	if(paginas_que_ocupa(dato_empaquetado.tamanio_data) <= obtener_marcos_vacios()){
		t_list* marcos_a_reservar = obtener_marcos_a_reservar(paginas_que_ocupa(dato_empaquetado.tamanio_data));
		modificar_tlb(dato_a_paginar->pcb.pid, marcos_a_reservar);
		setear_marcos_usados(marcos_a_reservar);
		setear_memoria(marcos_a_reservar, dato_empaquetado.data_empaquetada);
	}
}

void mostrar_array_marcos(){
	printf("\n*****ARRAY DE MARCOS*****\n");
	int aux =  obtener_tamanio_array_de_marcos();
	for(int i = 0; i<aux; i++){
		if(i<10)
			printf("--%d ", i);
		if(i>=10 && i<100)
			printf("-%d ", i);	
		if(i>=100 && i<1000)
			printf("%d ", i);
	}
	printf("\n");

	for(int a = 0; a<aux; a++){
		printf("--%d ", MARCOS_DISPONIBLES[a]);
	}
	printf("\n*************************\n\n");
}

void mostrar_tlb(){
	printf("\n*****TLB*****\n");
	void imprimir_un_valor(t_proceso_paginado* item_tlb){
		
		printf("PID: %d - ", item_tlb->pid);
		
		void imprimir_marco(int numero_marco){
		printf("%d ", numero_marco);
		}

		list_iterate(item_tlb->lista_de_marcos, imprimir_marco);
		printf("\n");
	}

	list_iterate(tlb, imprimir_un_valor);
	printf("*************************\n\n");
}

int main () {
	
	
	CANTIDAD_MARCOS = obtener_tamanio_array_de_marcos();
	MARCOS_DISPONIBLES = (uint32_t *) malloc( sizeof (uint32_t) * CANTIDAD_MARCOS); 
	
	
	//inicializo en 0 todo el bitmap

	for (int i = 0; i < CANTIDAD_MARCOS; i++) {
		MARCOS_DISPONIBLES[i] = 0;
	}

	
	MEMORIA = (void*) malloc (TAMANIO_MEMORIA);
	tlb = list_create();



	t_pcb mockpcb;
	mockpcb.pid= 2;
	mockpcb.tareas= 10;

	t_tcb* mocktcb = malloc(sizeof(t_tcb));
	mocktcb->estado= 1;
	mocktcb->posicion_x=2;
	mocktcb->posicion_y=3;
	mocktcb->proxima_instruccion=152;
	mocktcb->puntero_pcb= 0;
	mocktcb->tid= 19;

	t_list* tcblist = list_create();
	list_add(tcblist, mocktcb);
	list_add(tcblist, mocktcb);
	
	char* pepe;
	pepe = (char*) malloc(sizeof(char)*5);
	pepe = "Hola";

	estructura_administrativa_paginacion mockwrapeado;
	mockwrapeado.pcb = mockpcb;
	mockwrapeado.lista_de_tcb = tcblist;
	mockwrapeado.lista_de_tareas = pepe;

	//mostrar_array_marcos();
	//mostrar_tlb();

	paginar(&mockwrapeado);

	mostrar_array_marcos();
	mostrar_tlb();

	uint32_t b;
	memcpy(&b, MEMORIA + 46, 4);
	printf("El valor es %d\n",b);


	/*void* BOLSADEGATOS;
	BOLSADEGATOS = (void*) malloc (4);
	uint32_t ejemplo = 31;
	memcpy(BOLSADEGATOS, &ejemplo, sizeof(uint32_t));
	memcpy(MEMORIA, BOLSADEGATOS, 2);
	memcpy(MEMORIA + 2, BOLSADEGATOS + 2, 2);

	uint32_t lectura;

	memcpy(&lectura, MEMORIA, sizeof(uint32_t));*/

	/*char* pepe;
	pepe = (char*) malloc(sizeof(char)*5);
	pepe = "Hola";
	printf("El valor es %s\n",pepe);
	*/

	/*t_list* falopa = list_create();
	list_add(falopa, 1);
	list_add(falopa, 2);
	list_add(falopa, 4);
	

	printf("El valor es %d\n",list_size(falopa));*/

	/*t_pcb mockpcb;
	mockpcb.pid= 2;
	mockpcb.tareas= 10;

	t_tcb* mocktcb = malloc(sizeof(t_tcb));
	mocktcb->estado= 1;
	mocktcb->posicion_x=2;
	mocktcb->posicion_y=3;
	mocktcb->proxima_instruccion=152;
	mocktcb->puntero_pcb= 0;
	mocktcb->tid= 19;

	t_list* tcblist = list_create();
	list_add(tcblist, mocktcb);
	list_add(tcblist, mocktcb);
	
	char* pepe;
	pepe = (char*) malloc(sizeof(char)*5);
	pepe = "Hola";

	estructura_administrativa_paginacion mockwrapeado;
	mockwrapeado.pcb = mockpcb;
	mockwrapeado.lista_de_tcb = tcblist;
	mockwrapeado.lista_de_tareas = pepe;

	uint32_t a = 98;
	uint32_t b;
	char* pepes;
	pepes = (char*) malloc(sizeof(char)*5);
	dto_memoria testo;
	testo = empaquetar_data_paginacion(&mockwrapeado);
	memcpy(&b, testo.data_empaquetada+29, 4);
	printf("El valor es %d\n",b);

	MARCOS_DISPONIBLES[4] = 1;
	MARCOS_DISPONIBLES[5] = 1;
	MARCOS_DISPONIBLES[6] = 1;
	t_list* reservar = list_create();
	reservar = obtener_marcos_a_reservar(6);
	setear_marcos_usados(reservar);
	
	mostrar_array_marcos();

	setear_memoria(reservar, testo.data_empaquetada);
	memcpy(&b, MEMORIA + 4, 4);
	printf("El valor es %d\n",b);

	t_list* test = list_create();
	list_add(test, 1);
	t_list* test2 = list_create();
	modificar_tlb(17, test);
	modificar_tlb(8, reservar);

	mostrar_tlb();


	t_proceso_paginado* pantalla = list_get(tlb, 0);
	
	

	
	printf("El valor es %d\n",*pantalla->lista_de_marcos->head);*/

	
	return 0;
}