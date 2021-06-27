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

int obtener_tamanio_array_de_marcos_libres (int tamanio_paginas, int tamanio_memoria){
	if (tamanio_memoria >= tamanio_paginas) {
		printf("La cantidad de marcos disponibles es de %d marcos\n", tamanio_memoria/tamanio_paginas);
		printf("La cantidad de memoria que quedara inutilizable sera de %d bytes\n", tamanio_memoria%tamanio_paginas);
		
		return tamanio_memoria/tamanio_paginas;

	} else {
		printf("Hubo un error en la asignacion de marcos_libres");

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



void modificar_tlb (t_list* tlb, uint32_t id_proceso, t_list* marcos_utilizados) {
	
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


int main () {
	
	int TAMANIO_PAGINAS = 2;
	int TAMANIO_MEMORIA = 12;

	void* MEMORIA;
	uint32_t* marcos_libres;
	int cantidad_de_marcos = obtener_tamanio_array_de_marcos_libres(TAMANIO_PAGINAS, TAMANIO_MEMORIA);
	marcos_libres = (uint32_t *) malloc( sizeof (uint32_t) * cantidad_de_marcos); 
	
	
	//inicializo en 0 todo el bitmap

	for (int i = 0; i < cantidad_de_marcos; i++) {
		marcos_libres[i] = 0;
	}

	MEMORIA = (void*) malloc (TAMANIO_MEMORIA);
	t_list* tlb = list_create();

	void* BOLSADEGATOS;
	BOLSADEGATOS = (void*) malloc (4);
	uint32_t ejemplo = 31;
	memcpy(BOLSADEGATOS, &ejemplo, sizeof(uint32_t));
	memcpy(MEMORIA, BOLSADEGATOS, 2);
	memcpy(MEMORIA + 2, BOLSADEGATOS + 2, 2);

	uint32_t lectura;

	memcpy(&lectura, MEMORIA, sizeof(uint32_t));

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

	uint32_t a = 98;
	uint32_t b;
	char* pepes;
	pepes = (char*) malloc(sizeof(char)*5);
	dto_memoria testo;
	testo = empaquetar_data_paginacion(&mockwrapeado);
	memcpy(&b, testo.data_empaquetada+29, 4);
	printf("El valor es %d\n",b);
/*
	t_list* test = list_create();
	list_add(test, 1);
	t_list* test2 = list_create();
	list_add(test2, 2);
	modificar_tlb(tlb, 17, test);
	modificar_tlb(tlb, 17, test2);


	t_proceso_paginado* pantalla = list_get(tlb, 0);
	
	

	
	printf("El valor es %d\n",*pantalla->lista_de_marcos->head);

	*/
	return 0;
}