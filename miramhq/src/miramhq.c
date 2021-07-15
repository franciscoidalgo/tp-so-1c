#include "miramhq.h"


typedef struct {
    		uint32_t pid;
			t_list* lista_de_tids;
    		t_list* lista_de_marcos;
			t_list* lista_de_presencia;
		} t_tabla_proceso;

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
void *MEMORIA_VIRTUAL;
uint32_t* ESTADO_MARCOS;
uint32_t* ESTADO_MARCOS_VIRTUALES;
uint32_t* TIMESTAMP_MARCOS;
uint32_t COUNTER_LRU = 1;
uint32_t PUNTERO_CLOCK = 0;
uint32_t* BIT_CLOCK;
uint32_t MODO_DESALOJO = 0; //0 LRU 1 CLOCK
t_list* TABLA_DE_PAGINAS;
int TAMANIO_PAGINAS = 2;
int TAMANIO_MEMORIA = 60;
int TAMANIO_MEMORIA_VIRTUAL = 100;
int CANTIDAD_MARCOS;
int CANTIDAD_MARCOS_VIRTUALES;
int OFFSET = 15000;
	


int obtener_tamanio_array_de_marcos (){
	if (TAMANIO_MEMORIA >= TAMANIO_PAGINAS) {
		//printf("La cantidad de marcos disponibles es de %d marcos\n", TAMANIO_MEMORIA / TAMANIO_PAGINAS);
		//printf("La cantidad de memoria que quedara inutilizable sera de %d bytes\n", TAMANIO_MEMORIA % TAMANIO_PAGINAS);
		return TAMANIO_MEMORIA/TAMANIO_PAGINAS;
	} else {
		printf("Hubo un error en la asignacion de ESTADO_MARCOS");
		return 10;
	}
}

int obtener_tamanio_array_de_marcos_virtuales (){
	if (TAMANIO_MEMORIA_VIRTUAL >= TAMANIO_PAGINAS) {
		//printf("La cantidad de marcos disponibles es de %d marcos\n", TAMANIO_MEMORIA / TAMANIO_PAGINAS);
		//printf("La cantidad de memoria que quedara inutilizable sera de %d bytes\n", TAMANIO_MEMORIA % TAMANIO_PAGINAS);
		return TAMANIO_MEMORIA_VIRTUAL/TAMANIO_PAGINAS;
	} else {
		printf("Hubo un error en la asignacion de ESTADO_MARCOS_VIRTUALES");
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
		 memcpy(memaux + 8 + 21*i, &aux_tcb->tid, 4);
		 memcpy(memaux + 12 + 21*i, &aux_tcb->estado, 1);
		 memcpy(memaux + 13 + 21*i, &aux_tcb->posicion_x, 4);
		 memcpy(memaux + 17 + 21*i, &aux_tcb->posicion_y, 4);
		 memcpy(memaux + 21 + 21*i, &aux_tcb->proxima_instruccion, 4);
		 memcpy(memaux + 25 + 21*i, &aux_tcb->puntero_pcb, 4);
	}
	memcpy(memaux + 8 + cantidad_de_tripulantes * 21, &data_a_empaquetar->lista_de_tareas, largo_lista_de_tareas);
	dto_aux.data_empaquetada = memaux;

	return dto_aux;
}

void modificar_tlb (uint32_t id_proceso, t_list* lista_tids, t_list* marcos_utilizados) {
	
	bool find(void* element){
		t_tabla_proceso* aux = element;
		return aux->pid == id_proceso;
	}

		list_remove_and_destroy_by_condition(TABLA_DE_PAGINAS, find, free);
		t_tabla_proceso* dto = malloc(sizeof(t_tabla_proceso));
		dto->pid = id_proceso;
		dto->lista_de_marcos = marcos_utilizados;
		dto->lista_de_tids = lista_tids;
		t_list* lista_presencia;
		lista_presencia = list_create();
		int aux2 = list_size(marcos_utilizados);
		for(int i = 0; i<aux2; i++) {
			list_add(lista_presencia, 1);
		}	
		dto->lista_de_presencia = lista_presencia;
		list_add(TABLA_DE_PAGINAS, dto);
}

int obtener_marcos_vacios() {
	int marcos_vacios = 0;
	
	for(int i=0; i<CANTIDAD_MARCOS; i++) {
		if(ESTADO_MARCOS[i]==0) {
			marcos_vacios++;
		}
	} 
	return marcos_vacios;
}

int obtener_marcos_vacios_virtuales() {
	int marcos_vacios_virtuales = 0;
	
	for(int i=0; i<CANTIDAD_MARCOS_VIRTUALES; i++) {
		if(ESTADO_MARCOS_VIRTUALES[i]==0) {
			marcos_vacios_virtuales++;
		}
	} 
	return marcos_vacios_virtuales;
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
	list_iterate(lista_a_reservar, (void*)setear_una_pagina);
}

void pasar_un_marco_de_memoria(uint32_t marco_virtual, uint32_t marco_principal, bool sentido){ //false de memoria principal a virtual
	if(sentido==false){
		memcpy(MEMORIA_VIRTUAL + (marco_virtual*TAMANIO_PAGINAS), MEMORIA +(marco_principal*TAMANIO_PAGINAS), TAMANIO_PAGINAS);
	} else {
		memcpy(MEMORIA + (marco_principal*TAMANIO_PAGINAS), MEMORIA_VIRTUAL + (marco_virtual*TAMANIO_PAGINAS), TAMANIO_PAGINAS);
	}
}


t_list* obtener_marcos_a_reservar(int paginas_solicitadas){
	t_list* aux_list = list_create();
	int contador_array = 0;
	for(int i=0; i<paginas_solicitadas; i++) {
		if(ESTADO_MARCOS[contador_array]==0) {
			list_add(aux_list, contador_array);
		} else {
			i--;
		}
		contador_array++;
	} 
	return aux_list;
}

void setear_marco_como_usado(int numero_marco){
		ESTADO_MARCOS[numero_marco] = 1;
		TIMESTAMP_MARCOS[numero_marco] = COUNTER_LRU;
		COUNTER_LRU++;
		BIT_CLOCK[numero_marco] = 1;
}

void setear_marcos_usados(t_list* lista_a_reservar){
	list_iterate(lista_a_reservar, (void*)setear_marco_como_usado);
}

uint32_t indice_del_minimo_de_un_array(uint32_t* array, uint32_t tamanio_array){
	uint32_t min = array[0];
	uint32_t indice_min = 0;
	for(int i = 1; i<tamanio_array; i++) {
		if(min > array[i]){
			min = array[i];
			indice_min = i;
		}
	}
	return indice_min;
}

void reemplazar_marco_de_tabla_de_paginas(uint32_t marco_a_reemplazar, uint32_t nuevo_marco, int presencia) {
	t_tabla_proceso* aux2;
	uint32_t indice = 0;
	uint32_t aux3 = 0;

	bool find(void* element){
		t_tabla_proceso* aux = element;
		bool find2(void* num){
			return num == marco_a_reemplazar;
		}
		return list_any_satisfy(aux->lista_de_marcos, find2);
	}

	void obtener_indice_marco(uint32_t num){	
		if(num == marco_a_reemplazar){
			indice = aux3;
		}else{
			aux3++;
		}
	}

	aux2 = list_find(TABLA_DE_PAGINAS, find);
	list_iterate(aux2->lista_de_marcos, obtener_indice_marco);
	list_replace(aux2->lista_de_marcos, indice, nuevo_marco);
	switch (presencia){
	case 0:
		list_replace(aux2->lista_de_presencia, indice, 0);
		break;

	case 1:
		list_replace(aux2->lista_de_presencia, indice, 1);
		break;
	}
}

uint32_t obtener_marco_virtual_vacio(){
	int aux = 0;
	while (ESTADO_MARCOS_VIRTUALES[aux]!=0){
		aux++;
	}
	return aux;
}

void desalojar_un_marco(int marco_a_desalojar){
	if(ESTADO_MARCOS[marco_a_desalojar]==1){
		uint32_t marco_virtual = obtener_marco_virtual_vacio();
		pasar_un_marco_de_memoria(marco_virtual, marco_a_desalojar, false);
		ESTADO_MARCOS[marco_a_desalojar] = 0;
		ESTADO_MARCOS_VIRTUALES[marco_virtual] = 1;
		reemplazar_marco_de_tabla_de_paginas(marco_a_desalojar, marco_virtual + OFFSET,  0);
	}
}

t_list* obtener_marcos_a_desalojar(uint32_t numero_de_paginas_a_desalojar) {
	t_list* victimas = list_create();
	if(MODO_DESALOJO == 0){
		for(int i=0; i<numero_de_paginas_a_desalojar; i++){
			uint32_t marco_a_desalojar = indice_del_minimo_de_un_array(TIMESTAMP_MARCOS, CANTIDAD_MARCOS);
			 //guarda con los marcos sin usar ver aumentarles el timestamp antes por las dudas
			TIMESTAMP_MARCOS[marco_a_desalojar] = COUNTER_LRU;
			COUNTER_LRU++;
			list_add(victimas, marco_a_desalojar);
		}
	} else {
		for(int i=0; i<numero_de_paginas_a_desalojar; i++){
			while (BIT_CLOCK[PUNTERO_CLOCK]!= 0){
				BIT_CLOCK[PUNTERO_CLOCK] = 0;
				if (PUNTERO_CLOCK == CANTIDAD_MARCOS){
					PUNTERO_CLOCK = 0;
				} else {
					PUNTERO_CLOCK++;
				}
			}
			list_add(victimas, PUNTERO_CLOCK);
		}
	}
	return victimas;
}

void desalojar(uint32_t numero_de_paginas_a_desalojar) {
	t_list* marcos_a_desalojar = list_create();
	marcos_a_desalojar = obtener_marcos_a_desalojar(numero_de_paginas_a_desalojar);
	list_iterate(marcos_a_desalojar, desalojar_un_marco);
}


t_list* listar_tids(estructura_administrativa_paginacion* dato){
	int32_t obtener_tid(t_tcb* elem){
		return elem->tid;	
	}
	return list_map(dato->lista_de_tcb, obtener_tid);
}


void iniciar_patota (estructura_administrativa_paginacion* dato_a_paginar){
	dto_memoria dato_empaquetado = empaquetar_data_paginacion(dato_a_paginar);
	if(paginas_que_ocupa(dato_empaquetado.tamanio_data) <= obtener_marcos_vacios()){
		t_list* marcos_a_reservar = obtener_marcos_a_reservar(paginas_que_ocupa(dato_empaquetado.tamanio_data));
		modificar_tlb(dato_a_paginar->pcb.pid, listar_tids(dato_a_paginar), marcos_a_reservar);
		setear_marcos_usados(marcos_a_reservar);
		setear_memoria(marcos_a_reservar, dato_empaquetado.data_empaquetada);
	} else {
		if(paginas_que_ocupa(dato_empaquetado.tamanio_data) <= (obtener_marcos_vacios() + obtener_marcos_vacios_virtuales())) {
			desalojar(paginas_que_ocupa(dato_empaquetado.tamanio_data));
			t_list* marcos_a_reservar = obtener_marcos_a_reservar(paginas_que_ocupa(dato_empaquetado.tamanio_data));
			modificar_tlb(dato_a_paginar->pcb.pid, listar_tids(dato_a_paginar), marcos_a_reservar);
			setear_marcos_usados(marcos_a_reservar);
			setear_memoria(marcos_a_reservar, dato_empaquetado.data_empaquetada);
		}
	}
}



t_tabla_proceso* buscar_proceso(uint32_t id_proceso){
	bool buscarIdProceso(t_tabla_proceso* elem){
		return elem->pid == id_proceso;
	}

	return list_find(TABLA_DE_PAGINAS, buscarIdProceso);
}

uint32_t indice_de_tripulante(t_list* lista_de_ids, uint32_t id_tripulante){
	int indice = 0;
	int aux = 0;

	void buscarIndice(uint32_t elem){
		if(elem == id_tripulante){
			indice = aux;
		}
		aux++;
	}

	list_iterate(lista_de_ids, buscarIndice);
	return indice;
}

t_list* lista_marcos_en_virtual(t_list* lista_de_paginas, uint32_t id_proceso){
	t_tabla_proceso* aux = buscar_proceso(id_proceso);
	bool estan_en_virtual(uint32_t numero){
		return 0 == list_get(aux->lista_de_presencia, numero);			
	}

	uint32_t mapeo_a_marco(uint32_t elem){
		return list_get(aux->lista_de_marcos, elem);	
	}
	return list_map(list_filter(lista_de_paginas, estan_en_virtual), mapeo_a_marco);
}

void alojar(t_list* lista_marcos_en_virtual){
	void* memoriaaux;
	uint32_t aux = 0;
	uint32_t aux2 = 0;
	uint32_t tamanio_memoria_aux = list_size(lista_marcos_en_virtual);
	tamanio_memoria_aux = tamanio_memoria_aux * TAMANIO_PAGINAS;
	memoriaaux = (void*) malloc (tamanio_memoria_aux);
	void backupear_liberar(uint32_t num){
		memcpy(memoriaaux + (aux*TAMANIO_PAGINAS), MEMORIA_VIRTUAL + ((num - OFFSET)*TAMANIO_PAGINAS), TAMANIO_PAGINAS);
		ESTADO_MARCOS_VIRTUALES[num] = 0;
		aux ++;	
	}
	list_iterate(lista_marcos_en_virtual, backupear_liberar);
	desalojar(list_size(lista_marcos_en_virtual));
	t_list* marcos_a_reservar = obtener_marcos_a_reservar(list_size(lista_marcos_en_virtual));
	setear_marcos_usados(marcos_a_reservar);
	setear_memoria(marcos_a_reservar, memoriaaux);

	void corregir_tabla_paginas(uint32_t marco){
		reemplazar_marco_de_tabla_de_paginas(marco, list_get(marcos_a_reservar, aux2), 1);
		aux2++;
	}
	
	list_iterate(lista_marcos_en_virtual, corregir_tabla_paginas);
}

void setear_nueva_posicion(t_list* marcos_a_cambiar, uint32_t posx, uint32_t posy, uint32_t desplazamiento){
	void* memoria_aux;
	memoria_aux = (void*) malloc (8 + desplazamiento);
	uint32_t aux = list_get(marcos_a_cambiar, 0);
	memcpy(memoria_aux, MEMORIA + aux * TAMANIO_PAGINAS, desplazamiento);
	memcpy(memoria_aux + desplazamiento, &posx, desplazamiento);
	memcpy(memoria_aux + desplazamiento + 4, &posy, desplazamiento);
	setear_memoria(marcos_a_cambiar, memoria_aux);
}

void cambiar_ubicacion_tripulante(uint32_t id_proceso, uint32_t id_tripulante, uint32_t nueva_posx, uint32_t nueva_posy){
	t_tabla_proceso* aux = buscar_proceso(id_proceso);
	uint32_t tripulante_logico = indice_de_tripulante(aux->lista_de_tids, id_tripulante);
	uint32_t direccion_logica = 8 + 5 + (21*tripulante_logico);
	t_list* paginas_a_cambiar = list_create();
	 
	if(direccion_logica/TAMANIO_PAGINAS == (direccion_logica+8)/TAMANIO_PAGINAS){
		list_add(paginas_a_cambiar, direccion_logica/TAMANIO_PAGINAS);
	}

	for (int i = direccion_logica/TAMANIO_PAGINAS; i<=(direccion_logica+8)/TAMANIO_PAGINAS; i++){
		list_add(paginas_a_cambiar, i);
	}
	uint32_t direccion_logica_inicio = list_get(paginas_a_cambiar, 0);
	direccion_logica_inicio = direccion_logica_inicio * TAMANIO_PAGINAS;

	t_list* marcos_en_virtual = lista_marcos_en_virtual(paginas_a_cambiar, id_proceso);

	if(list_size(marcos_en_virtual)!=0){
		alojar(marcos_en_virtual);
	}
	
	t_list* marcos_a_cambiar;
	uint32_t mapeo_de_marco(uint32_t elem){
		return list_get(aux->lista_de_marcos, elem);	
	}
	marcos_a_cambiar = list_map(paginas_a_cambiar, mapeo_de_marco);

	printf("El numerillo es %d\n", direccion_logica - direccion_logica_inicio);

	setear_nueva_posicion(marcos_a_cambiar, nueva_posx, nueva_posy, direccion_logica - direccion_logica_inicio);

	/*printf("El numerillo es %d\n", list_get(paginas_a_cambiar, 4)); */
}

void mostrar_array_marcos(){
	printf("\n*****ARRAY DE MARCOS*****\n");
	for(int i = 0; i<CANTIDAD_MARCOS; i++){
		if(i<10)
			printf("--%d ", i);
		if(i>=10 && i<100)
			printf("-%d ", i);	
		if(i>=100 && i<1000)
			printf("%d ", i);
	}
	printf("\n");

	for(int a = 0; a<CANTIDAD_MARCOS; a++){
		printf("--%d ", ESTADO_MARCOS[a]);
	}
	printf("\n*************************\n\n");
}

void mostrar_array_marcos_virtuales(){
	printf("\n*****ARRAY DE MARCOS VIRTUALES*****\n");
	for(int i = 0; i<CANTIDAD_MARCOS_VIRTUALES; i++){
		if(i<10)
			printf("--%d ", i);
		if(i>=10 && i<100)
			printf("-%d ", i);	
		if(i>=100 && i<1000)
			printf("%d ", i);
	}
	printf("\n");

	for(int a = 0; a<CANTIDAD_MARCOS_VIRTUALES; a++){
		printf("--%d ", ESTADO_MARCOS_VIRTUALES[a]);
	}
	printf("\n*************************\n\n");
}


void mostrar_tabla_de_paginas(){
	
	printf("\n*****TABLA DE PAGINAS*****\n");
	void imprimir_un_valor(t_tabla_proceso* item_tlb){
		int i = 0;
		printf("PID: %d - ", item_tlb->pid);

		printf("TIDS: ", item_tlb->pid);
		void imprimir_marco2(int num){
			printf("%d ", num);
		}

		list_iterate(item_tlb->lista_de_tids, imprimir_marco2);
		
		printf("- ");
		void imprimir_marco(int num){
		printf("%d/", num);
		printf("%d ", list_get(item_tlb->lista_de_presencia, i));
		i++;
		}

		list_iterate(item_tlb->lista_de_marcos, imprimir_marco);
		printf("\n");
	}

	list_iterate(TABLA_DE_PAGINAS, imprimir_un_valor);
	printf("********************************\n\n");
}

int main () {
	
	CANTIDAD_MARCOS = obtener_tamanio_array_de_marcos();
	ESTADO_MARCOS = (uint32_t *) malloc( sizeof (uint32_t) * CANTIDAD_MARCOS);
	TIMESTAMP_MARCOS = (uint32_t *) malloc( sizeof (uint32_t) * CANTIDAD_MARCOS);
	BIT_CLOCK =  (uint32_t *) malloc( sizeof (uint32_t) * CANTIDAD_MARCOS);
	CANTIDAD_MARCOS_VIRTUALES = obtener_tamanio_array_de_marcos_virtuales();
	ESTADO_MARCOS_VIRTUALES = (uint32_t *) malloc( sizeof (uint32_t) * CANTIDAD_MARCOS_VIRTUALES);
	
	//inicializo en 0 todo el bitmap

	for (int i = 0; i < CANTIDAD_MARCOS; i++) {
		ESTADO_MARCOS[i] = 0;
	}

	for (int i = 0; i < CANTIDAD_MARCOS_VIRTUALES; i++) {
		ESTADO_MARCOS_VIRTUALES[i] = 0;
	}

	MEMORIA = (void*) malloc (TAMANIO_MEMORIA);
	TABLA_DE_PAGINAS = list_create();


	int fd = open("cfg/virtualmemory.txt",O_RDWR , S_IRUSR | S_IWUSR);
	struct stat sb;

	ftruncate(fd, TAMANIO_MEMORIA_VIRTUAL);

	if(fstat(fd, &sb) == -1) {
		perror("No pude obtener el tamaño del archivo.\n");
	}

	printf("Tamaño del Archivo: %ld\n",sb.st_size);

	MEMORIA_VIRTUAL = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	t_pcb mockpcb;
	mockpcb.pid= 2;
	mockpcb.tareas= 10;

	t_tcb* mocktcb = malloc(sizeof(t_tcb));
	mocktcb->estado= '1';
	mocktcb->posicion_x=2;
	mocktcb->posicion_y=3;
	mocktcb->proxima_instruccion=152;
	mocktcb->puntero_pcb= 0;
	mocktcb->tid= 19;

	t_tcb* mocktcb3 = malloc(sizeof(t_tcb));
	mocktcb3->estado= '1';
	mocktcb3->posicion_x=2;
	mocktcb3->posicion_y=3;
	mocktcb3->proxima_instruccion=152;
	mocktcb3->puntero_pcb= 0;
	mocktcb3->tid= 3;

	t_list* tcblist = list_create();
	list_add(tcblist, mocktcb);
	list_add(tcblist, mocktcb3);
	
	char* pepe;
	pepe = (char*) malloc(sizeof(char)*5);
	pepe = "Hola";

	estructura_administrativa_paginacion mockwrapeado;
	mockwrapeado.pcb = mockpcb;
	mockwrapeado.lista_de_tcb = tcblist;
	mockwrapeado.lista_de_tareas = pepe;

	t_pcb mockpcb2;
	mockpcb2.pid= 4;
	mockpcb2.tareas= 10;

	t_tcb* mocktcb2 = malloc(sizeof(t_tcb));
	mocktcb2->estado= 1;
	mocktcb2->posicion_x=2;
	mocktcb2->posicion_y=3;
	mocktcb2->proxima_instruccion=152;
	mocktcb2->puntero_pcb= 0;
	mocktcb2->tid= 2;

	t_list* tcblist2 = list_create();
	list_add(tcblist2, mocktcb2);
	
	char* pepe2;
	pepe2 = (char*) malloc(sizeof(char)*5);
	pepe2 = "Hola";

	estructura_administrativa_paginacion mockwrapeado2;
	mockwrapeado2.pcb = mockpcb2;
	mockwrapeado2.lista_de_tcb = tcblist2;
	mockwrapeado2.lista_de_tareas = pepe2;


	iniciar_patota(&mockwrapeado);
	

	mostrar_array_marcos();
	mostrar_tabla_de_paginas();
	mostrar_array_marcos_virtuales();

	char b;
	//memcpy(&b, MEMORIA+17, 4);
	//printf("El valor es %d\n",b);

	cambiar_ubicacion_tripulante(2, 2, 8, 8);

	memcpy(&b, MEMORIA+12, 1);
	printf("El valor es %c\n",b);
	
	return 0;
}