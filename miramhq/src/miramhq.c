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
			uint32_t tamanio_data;			
			void* data_empaquetada; 		
		} dto_memoria;

typedef struct{
			uint32_t marco;			
			char* estado;
			uint32_t proceso;
			uint32_t pagina; 		
		} dump_memoria;

typedef struct{
			uint32_t pid;			
			uint32_t tid;
			char mapid;
		} t_mapa;



void* MEMORIA;
void *MEMORIA_VIRTUAL;
uint32_t* ESTADO_MARCOS;
uint32_t* ESTADO_MARCOS_VIRTUALES;
uint32_t* TIMESTAMP_MARCOS;
uint32_t COUNTER_LRU = 1;
uint32_t PUNTERO_CLOCK = 0;
uint32_t* ARRAY_BIT_USO;
uint32_t MODO_DESALOJO = 0; //0 LRU 1 CLOCK
t_list* TABLA_DE_PAGINAS;
uint32_t TAMANIO_PAGINAS = 32;
uint32_t TAMANIO_MEMORIA = 128;
uint32_t TAMANIO_MEMORIA_VIRTUAL = 16384;
uint32_t CANTIDAD_MARCOS;
uint32_t CANTIDAD_MARCOS_VIRTUALES;
uint32_t OFFSET = 15000;

char ID_MAPA = 'A';
t_list* TABLA_DE_MAPA;
	


uint32_t obtener_tamanio_array_de_marcos (){
	if (TAMANIO_MEMORIA >= TAMANIO_PAGINAS) {
		//printf("La cantidad de marcos disponibles es de %d marcos\n", TAMANIO_MEMORIA / TAMANIO_PAGINAS);
		//printf("La cantidad de memoria que quedara inutilizable sera de %d bytes\n", TAMANIO_MEMORIA % TAMANIO_PAGINAS);
		return TAMANIO_MEMORIA/TAMANIO_PAGINAS;
	} else {
		printf("Hubo un error en la asignacion de ESTADO_MARCOS");
		return 10;
	}
}

uint32_t obtener_tamanio_array_de_marcos_virtuales (){
	if (TAMANIO_MEMORIA_VIRTUAL >= TAMANIO_PAGINAS) {
		//printf("La cantidad de marcos disponibles es de %d marcos\n", TAMANIO_MEMORIA / TAMANIO_PAGINAS);
		//printf("La cantidad de memoria que quedara inutilizable sera de %d bytes\n", TAMANIO_MEMORIA % TAMANIO_PAGINAS);
		return TAMANIO_MEMORIA_VIRTUAL/TAMANIO_PAGINAS;
	} else {
		printf("Hubo un error en la asignacion de ESTADO_MARCOS_VIRTUALES");
		return 10;
	}
}

uint32_t redondear_para_arriba (uint32_t numero, uint32_t divisor) {
	if (numero%divisor!= 0){
		return numero/divisor+1;
	} else {
		return numero/divisor;
	}
}

dto_memoria empaquetar_data_paginacion (estructura_administrativa_paginacion* data_a_empaquetar){
	dto_memoria dto_aux;

	uint32_t aux_tamanio = 0;
	uint32_t cantidad_de_tripulantes = list_size(data_a_empaquetar->lista_de_tcb);
	uint32_t largo_lista_de_tareas = strlen(data_a_empaquetar->lista_de_tareas) + 1;
	aux_tamanio += 8;
	aux_tamanio += cantidad_de_tripulantes * 21;
	data_a_empaquetar->pcb.tareas = aux_tamanio;
	aux_tamanio += largo_lista_de_tareas;
	
	dto_aux.tamanio_data = aux_tamanio;
	void* memaux;
	memaux = (void*) malloc (redondear_para_arriba(aux_tamanio, TAMANIO_PAGINAS)*TAMANIO_PAGINAS);
	memcpy(memaux, &data_a_empaquetar->pcb, 8);
	for (uint32_t i = 0; i < cantidad_de_tripulantes; i++ ) {
		 t_tcb* aux_tcb = list_get(data_a_empaquetar->lista_de_tcb, i);
		 memcpy(memaux + 8 + 21*i, &aux_tcb->tid, 4);
		 memcpy(memaux + 12 + 21*i, &aux_tcb->estado, 1);
		 memcpy(memaux + 13 + 21*i, &aux_tcb->posicion_x, 4);
		 memcpy(memaux + 17 + 21*i, &aux_tcb->posicion_y, 4);
		 memcpy(memaux + 21 + 21*i, &aux_tcb->proxima_instruccion, 4);
		 memcpy(memaux + 25 + 21*i, &aux_tcb->puntero_pcb, 4);
	}
	memcpy(memaux + 8 + cantidad_de_tripulantes * 21, data_a_empaquetar->lista_de_tareas, largo_lista_de_tareas);
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
		uint32_t aux2 = list_size(marcos_utilizados);
		for(uint32_t i = 0; i<aux2; i++) {
			list_add(lista_presencia, 1);
		}	
		dto->lista_de_presencia = lista_presencia;
		list_add(TABLA_DE_PAGINAS, dto);
}

uint32_t obtener_marcos_vacios() {
	uint32_t marcos_vacios = 0;
	
	for(uint32_t i=0; i<CANTIDAD_MARCOS; i++) {
		if(ESTADO_MARCOS[i]==0) {
			marcos_vacios++;
		}
	} 
	return marcos_vacios;
}

uint32_t obtener_marcos_vacios_virtuales() {
	uint32_t marcos_vacios_virtuales = 0;
	
	for(uint32_t i=0; i<CANTIDAD_MARCOS_VIRTUALES; i++) {
		if(ESTADO_MARCOS_VIRTUALES[i]==0) {
			marcos_vacios_virtuales++;
		}
	} 
	return marcos_vacios_virtuales;
}

uint32_t paginas_que_ocupa(uint32_t cantidad_bytes){
	uint32_t aux = cantidad_bytes / TAMANIO_PAGINAS;
	if (cantidad_bytes % TAMANIO_PAGINAS != 0) {
		aux ++;
	}
	return aux;
}

void setear_memoria(t_list* lista_a_reservar, void* data_empaquetada){
	uint32_t aux = 0;
	void setear_una_pagina(uint32_t numero_marco){
		memcpy(MEMORIA + (numero_marco * TAMANIO_PAGINAS), data_empaquetada + aux, TAMANIO_PAGINAS);
		aux = TAMANIO_PAGINAS + aux;
	}
	list_iterate(lista_a_reservar, (void*)setear_una_pagina);
	free(data_empaquetada);
}

void pasar_un_marco_de_memoria(uint32_t marco_virtual, uint32_t marco_principal, bool sentido){
	if(sentido==false){	
		memcpy(MEMORIA_VIRTUAL + (marco_virtual*TAMANIO_PAGINAS), MEMORIA +(marco_principal*TAMANIO_PAGINAS), TAMANIO_PAGINAS);
	} else {
		memcpy(MEMORIA + (marco_principal*TAMANIO_PAGINAS), MEMORIA_VIRTUAL + (marco_virtual*TAMANIO_PAGINAS), TAMANIO_PAGINAS);
	}
}


t_list* obtener_marcos_a_reservar(uint32_t paginas_solicitadas){
	t_list* aux_list = list_create();
	uint32_t contador_array = 0;
	for(uint32_t i=0; i<paginas_solicitadas; i++) {
		if(ESTADO_MARCOS[contador_array]==0) {
			list_add(aux_list, contador_array);
		} else {
			i--;
		}
		contador_array++;
	} 
	return aux_list;
}

void setear_marco_como_usado(uint32_t numero_marco){
		ESTADO_MARCOS[numero_marco] = 1;
		TIMESTAMP_MARCOS[numero_marco] = COUNTER_LRU;
		COUNTER_LRU++;
		ARRAY_BIT_USO[numero_marco] = 1;
}

void setear_marcos_usados(t_list* lista_a_reservar){
	list_iterate(lista_a_reservar, (void*)setear_marco_como_usado);
}

uint32_t indice_del_minimo_de_un_array(uint32_t* array, uint32_t tamanio_array){
	uint32_t min = array[0];
	uint32_t indice_min = 0;
	for(uint32_t i = 1; i<tamanio_array; i++) {
		if(min > array[i]){
			min = array[i];
			indice_min = i;
		}
	}
	return indice_min;
}



uint32_t obtener_proceso_de_marco(uint32_t marco){
	t_tabla_proceso* aux;
	bool find(void* element){
		t_tabla_proceso* aux = element;
		bool find2(void* num){
			return num == marco;
		}
		return list_any_satisfy(aux->lista_de_marcos, find2);
	}
	aux = list_find(TABLA_DE_PAGINAS, find);
	return aux->pid;
}

void reemplazar_marco_de_tabla_de_paginas(uint32_t marco_a_reemplazar, uint32_t nuevo_marco, uint32_t presencia) {
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
	uint32_t aux = 0;
	while (ESTADO_MARCOS_VIRTUALES[aux]!=0){
		aux++;
	}
	return aux;
}

void desalojar_un_marco(uint32_t marco_a_desalojar){
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
		for(uint32_t i=0; i<numero_de_paginas_a_desalojar; i++){
			uint32_t marco_a_desalojar = indice_del_minimo_de_un_array(TIMESTAMP_MARCOS, CANTIDAD_MARCOS);
			TIMESTAMP_MARCOS[marco_a_desalojar] = COUNTER_LRU;
			COUNTER_LRU++;
			list_add(victimas, marco_a_desalojar);
		}
	} else {
		for(uint32_t i=0; i<numero_de_paginas_a_desalojar; i++){
			while (ARRAY_BIT_USO[PUNTERO_CLOCK]!= 0){
				ARRAY_BIT_USO[PUNTERO_CLOCK] = 0;
				PUNTERO_CLOCK ++;

				if (PUNTERO_CLOCK == CANTIDAD_MARCOS){
					PUNTERO_CLOCK = 0;
				} 
			}
			list_add(victimas, PUNTERO_CLOCK);
			PUNTERO_CLOCK++;
			if (PUNTERO_CLOCK == CANTIDAD_MARCOS){
					PUNTERO_CLOCK = 0;
			} 
		}
	}
	return victimas;
}

void desalojar(uint32_t numero_de_paginas_a_desalojar) {
	if(numero_de_paginas_a_desalojar!=0){
		t_list* marcos_a_desalojar;
		marcos_a_desalojar = obtener_marcos_a_desalojar(numero_de_paginas_a_desalojar);
		list_iterate(marcos_a_desalojar, desalojar_un_marco);
		list_destroy(marcos_a_desalojar);
	}
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
	uint32_t indice = 0;
	uint32_t aux = 0;

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
	t_list* lista_de_marcos = list_create();
	uint32_t mapeo_a_marco_virtual(uint32_t pagina){
		if(0 == list_get(aux->lista_de_presencia, pagina)){
		uint32_t marco = list_get(aux->lista_de_marcos, pagina);
		list_add(lista_de_marcos, marco);
		}
	}
	list_iterate(lista_de_paginas, mapeo_a_marco_virtual);
	return lista_de_marcos;
}

void setear_nueva_posicion(t_list* marcos_a_cambiar, uint32_t posx, uint32_t posy, uint32_t desplazamiento){
	void* memoria_aux;
	uint32_t tamanio_a_alojar = list_size(marcos_a_cambiar)*TAMANIO_PAGINAS;
	uint32_t primer_marco = list_get(marcos_a_cambiar, 0);
	uint32_t ultimo_marco = list_get(marcos_a_cambiar, list_size(marcos_a_cambiar) - 1);

	memoria_aux = (void*) malloc (tamanio_a_alojar);
	memcpy(memoria_aux, MEMORIA + primer_marco * TAMANIO_PAGINAS, desplazamiento);
	memcpy(memoria_aux + desplazamiento, &posx, 4);
	memcpy(memoria_aux + desplazamiento + 4, &posy, 4);
	memcpy(memoria_aux + desplazamiento + 8, MEMORIA + ultimo_marco*TAMANIO_PAGINAS + (desplazamiento + 8)%TAMANIO_PAGINAS, tamanio_a_alojar-desplazamiento-8);
	setear_memoria(marcos_a_cambiar, memoria_aux);
}

void cambiar_ubicacion_tripulante(uint32_t id_proceso, uint32_t id_tripulante, uint32_t nueva_posx, uint32_t nueva_posy){
	t_tabla_proceso* aux = buscar_proceso(id_proceso);
	uint32_t tripulante_logico = indice_de_tripulante(aux->lista_de_tids, id_tripulante);
	uint32_t direccion_logica = 8 + 5 + (21*tripulante_logico);
	t_list* paginas_a_cambiar = list_create();

	 
	if(direccion_logica/TAMANIO_PAGINAS == (direccion_logica+8)/TAMANIO_PAGINAS){
		list_add(paginas_a_cambiar, direccion_logica/TAMANIO_PAGINAS);
	} else {
		for (uint32_t i = direccion_logica/TAMANIO_PAGINAS; i<=(direccion_logica+8)/TAMANIO_PAGINAS; i++){
			list_add(paginas_a_cambiar, i);
		}
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
	setear_nueva_posicion(marcos_a_cambiar, nueva_posx, nueva_posy, direccion_logica - direccion_logica_inicio);
	list_destroy(marcos_en_virtual);
	list_destroy(marcos_a_cambiar);
	list_destroy(paginas_a_cambiar);
}

t_list* obtener_marcos_de_paginas(t_list* lista_de_marcos, uint32_t pagina_inicio, uint32_t pagina_fin){
	t_list* aux = list_create();
	for (uint32_t i=pagina_inicio; i<=pagina_fin; i++){
		list_add(aux, list_get(lista_de_marcos, i));
	}
	return aux;
}

void liberar_marcos(t_list* lista_marcos_borrado){
	void liberar_marco(uint32_t marco){
		if(marco>OFFSET){
			ESTADO_MARCOS_VIRTUALES[marco-OFFSET] = 0;
		} else {
			ESTADO_MARCOS[marco]=0;
		}
	}
	
	list_iterate(lista_marcos_borrado, liberar_marco);
}

void liberar_tabla(t_list* marcos, t_list* presencia, uint32_t pagina_inicio, uint32_t pagina_fin){
	for (uint32_t i=pagina_inicio; i<=pagina_fin; i++){
		list_remove(marcos, pagina_inicio);
		list_remove(presencia, pagina_inicio);
	}
}


uint32_t modificar_direccion_tareas(t_list* marcos_a_modificar, uint32_t dezplazamiento) {
	//devuelve direccion logica anterior de tareas
	uint32_t cantidad_de_marcos = list_size(marcos_a_modificar);
	void* memoria_auxi;
	memoria_auxi = (void*) malloc (cantidad_de_marcos*TAMANIO_PAGINAS);
	for(uint32_t i=0; i<cantidad_de_marcos; i++){
		uint32_t aux = list_get(marcos_a_modificar, i);
		memcpy(memoria_auxi+(i*TAMANIO_PAGINAS), MEMORIA + (aux*TAMANIO_PAGINAS), TAMANIO_PAGINAS);
	}
	uint32_t direccion_logica_tareas;
	memcpy(&direccion_logica_tareas, memoria_auxi + dezplazamiento, 4);
	direccion_logica_tareas = direccion_logica_tareas - 21;
	memcpy(memoria_auxi + dezplazamiento, &direccion_logica_tareas, 4);
	setear_memoria(marcos_a_modificar, memoria_auxi);
	return (direccion_logica_tareas + 21);
}

void necesito_en_ppal(uint32_t direccion_logica_inicio, uint32_t direccion_logica_fin, uint32_t id_proceso){
	t_tabla_proceso* aux = buscar_proceso(id_proceso);
	uint32_t pagina_inicio_check = direccion_logica_inicio/TAMANIO_PAGINAS;
	uint32_t pagina_fin_check = redondear_para_arriba (direccion_logica_fin, TAMANIO_PAGINAS);
	t_list* marcos_a_alojar = list_create();
	for(uint32_t i=pagina_inicio_check; i< pagina_fin_check; i++){
		uint32_t marco_aux = list_get(aux->lista_de_marcos, i);
		if(marco_aux>=OFFSET){
			list_add(marcos_a_alojar, marco_aux);
		}
	}
	alojar(marcos_a_alojar);
	list_destroy(marcos_a_alojar);
}

void reemplazar_marco_de_tabla_por_indice(uint32_t id_proceso, uint32_t nuevo_marco, uint32_t numero_pagina, uint32_t presencia) {
	t_tabla_proceso* aux2;
	aux2 = buscar_proceso(id_proceso);
	list_replace(aux2->lista_de_marcos, numero_pagina, nuevo_marco);
	switch (presencia){
	case 0:
		list_replace(aux2->lista_de_presencia, numero_pagina, 0);
		break;

	case 1:
		list_replace(aux2->lista_de_presencia, numero_pagina, 1);
		break;
	}
}

uint32_t obtener_indice_de_marco(uint32_t marco, uint32_t id_proceso){
	t_tabla_proceso* aux = buscar_proceso(id_proceso);
	uint32_t aux2 = 0;
	uint32_t indice;

	void buscarIndicex(uint32_t elem){
		if(elem == marco){
			indice = aux2;
		}
		aux2++;
	}

	list_iterate(aux->lista_de_marcos, buscarIndicex);
	return indice;
}

void alojar(t_list* lista_marcos_en_virtual){
	void* memoriaaux;
	uint32_t aux = 0;
	uint32_t aux2 = 0;
	uint32_t tamanio_memoria_aux = list_size(lista_marcos_en_virtual);
	t_list* paginas_a_corregir=list_create();
	t_list* procesos_a_corregir=list_create();
	void cargar_listas(marco_virtual){
		uint32_t proceso = obtener_proceso_de_marco(marco_virtual);
		list_add(procesos_a_corregir, proceso);
		list_add(paginas_a_corregir, obtener_indice_de_marco(marco_virtual, proceso));
	}
	list_iterate(lista_marcos_en_virtual, cargar_listas);
	tamanio_memoria_aux = tamanio_memoria_aux * TAMANIO_PAGINAS;
	memoriaaux = (void*) malloc (tamanio_memoria_aux);
	void backupear_liberar(uint32_t num){
		memcpy(memoriaaux + (aux*TAMANIO_PAGINAS), MEMORIA_VIRTUAL + ((num - OFFSET)*TAMANIO_PAGINAS), TAMANIO_PAGINAS);
		ESTADO_MARCOS_VIRTUALES[num-OFFSET] = 0;
		aux ++;	
	}
	list_iterate(lista_marcos_en_virtual, backupear_liberar);
	desalojar(list_size(lista_marcos_en_virtual));
	t_list* marcos_a_reservar = obtener_marcos_a_reservar(list_size(lista_marcos_en_virtual));
	setear_marcos_usados(marcos_a_reservar);
	setear_memoria(marcos_a_reservar, memoriaaux);
	void corregir_tabla_paginas(uint32_t marco){
		reemplazar_marco_de_tabla_por_indice(list_get(procesos_a_corregir, aux2), list_get(marcos_a_reservar, aux2), list_get(paginas_a_corregir, aux2), 1);
		aux2++;
	}
	list_iterate(lista_marcos_en_virtual, corregir_tabla_paginas);
	list_destroy(paginas_a_corregir);
	list_destroy(procesos_a_corregir);
	list_destroy(marcos_a_reservar);
}

t_list* obtener_marcos_segun_direccion_logica(uint32_t direccion_logica_inicio, uint32_t direccion_logica_fin, t_list* lista_de_marcos_completa){
	t_list* aux = list_create();
	uint32_t aux2 = 0;
	uint32_t pagina_inicio = direccion_logica_inicio / TAMANIO_PAGINAS;
	uint32_t pagina_fin = redondear_para_arriba(direccion_logica_fin, TAMANIO_PAGINAS);
	void filtrar_marcos(marco){
		if(aux2>=pagina_inicio && aux2<pagina_fin){
			list_add(aux, marco);
		}
		aux2++;
	}
	list_iterate(lista_de_marcos_completa, filtrar_marcos);
	return aux;
}

uint32_t obtener_indice_del_proceso(uint32_t pid){
	uint32_t aux = 0;
	uint32_t indice = 0;
	void buscar_id_proceso(t_tabla_proceso* tabla_proceso){
		if(tabla_proceso->pid==pid){
			indice = aux;
		}
		aux++;
	}
	list_iterate(TABLA_DE_PAGINAS, buscar_id_proceso);
	return indice;
}

uint32_t tamanio_lista_tareas(uint32_t id_proceso, uint32_t direccion_logica_tareas){
	t_tabla_proceso* aux = buscar_proceso(id_proceso);
	uint32_t ultima_direccion_logica = list_size(aux->lista_de_marcos) * TAMANIO_PAGINAS;

	t_list* marcos_de_lista_de_tareas;
	necesito_en_ppal(direccion_logica_tareas, ultima_direccion_logica, id_proceso);
	marcos_de_lista_de_tareas = obtener_marcos_segun_direccion_logica(direccion_logica_tareas, ultima_direccion_logica, aux->lista_de_marcos);
	
	uint32_t offset_tareas;
	offset_tareas = list_get(marcos_de_lista_de_tareas, 0);
	offset_tareas = offset_tareas * TAMANIO_PAGINAS;
	offset_tareas = offset_tareas + (direccion_logica_tareas%TAMANIO_PAGINAS);
	void* lista_tareas;
	lista_tareas = (void*) malloc (ultima_direccion_logica-direccion_logica_tareas);
	memcpy(lista_tareas, MEMORIA + offset_tareas, TAMANIO_PAGINAS - (direccion_logica_tareas%TAMANIO_PAGINAS));
	for(uint32_t i = 1; i < list_size(marcos_de_lista_de_tareas); i++) {
		uint32_t aux = list_get(marcos_de_lista_de_tareas, i);
		aux = aux * TAMANIO_PAGINAS;
		memcpy(lista_tareas +  TAMANIO_PAGINAS - (direccion_logica_tareas%TAMANIO_PAGINAS) + ((i-1) * TAMANIO_PAGINAS), MEMORIA + aux, TAMANIO_PAGINAS);
	}
	uint32_t tam_lista_tareas = strlen(lista_tareas)+1;
	free(lista_tareas);	
	list_destroy(marcos_de_lista_de_tareas);
	return tam_lista_tareas;
}

t_list* compactar_tripulante(uint32_t id_proceso, uint32_t direccion_logica_inicio, uint32_t ultima_direccion_logica, uint32_t lista_tareas){
	t_tabla_proceso* aux2 = buscar_proceso(id_proceso);
	t_list* lista_de_marcos_aux;
	uint32_t aux = 0;
	necesito_en_ppal(direccion_logica_inicio, ultima_direccion_logica, aux2->pid);
	lista_de_marcos_aux = obtener_marcos_segun_direccion_logica(direccion_logica_inicio, ultima_direccion_logica, aux2->lista_de_marcos);
	uint32_t tamanio_a_reservar = list_size(lista_de_marcos_aux)*TAMANIO_PAGINAS;
	void* memoria_aux;
	memoria_aux = (void*) malloc (tamanio_a_reservar);
	void copiar_a_memoria(uint32_t numero_marco){
		memcpy(memoria_aux + aux*TAMANIO_PAGINAS, MEMORIA + numero_marco*TAMANIO_PAGINAS, TAMANIO_PAGINAS);
		aux++;
	}
	list_iterate(lista_de_marcos_aux, copiar_a_memoria);
	
	uint32_t direccion_logica_inicio_real = (direccion_logica_inicio/TAMANIO_PAGINAS)*TAMANIO_PAGINAS;
	uint32_t desplazamiento = direccion_logica_inicio-direccion_logica_inicio_real;
	uint32_t cantidad_a_copiar = list_size(aux2->lista_de_tids);
	cantidad_a_copiar = (cantidad_a_copiar - 1) * 21 + lista_tareas;  
	
	memcpy(memoria_aux+desplazamiento, memoria_aux+desplazamiento+21, cantidad_a_copiar);

	uint32_t paginas_antes = redondear_para_arriba((8 + list_size(aux2->lista_de_tids) * 21 + lista_tareas), TAMANIO_PAGINAS);
	uint32_t paginas_ahora = redondear_para_arriba((8 + list_size(aux2->lista_de_tids) * 21 - 21 + lista_tareas), TAMANIO_PAGINAS); 
	uint32_t paginas_a_agarrar = list_size(lista_de_marcos_aux) - (paginas_antes-paginas_ahora);

	t_list* marcos_a_memoria;
	marcos_a_memoria = list_take(lista_de_marcos_aux, paginas_a_agarrar);

	setear_memoria(marcos_a_memoria, memoria_aux);

	list_destroy(marcos_a_memoria);

	t_list* marcos_a_borrar;
	marcos_a_borrar = list_create();
	for(uint32_t i = paginas_a_agarrar; i<list_size(lista_de_marcos_aux); i++){
		uint32_t auxiliar = list_get(lista_de_marcos_aux, i);
		list_add(marcos_a_borrar, auxiliar);
	}

	list_destroy(lista_de_marcos_aux);

	return marcos_a_borrar;
}


void expulsar_tripulante(uint32_t id_proceso, uint32_t id_tripulante){
	
	t_tabla_proceso* aux = buscar_proceso(id_proceso);

	if(list_size(aux->lista_de_tids)==1){
		liberar_marcos(aux->lista_de_marcos);
		list_destroy(aux->lista_de_marcos);
		list_destroy(aux->lista_de_presencia);
		list_destroy(aux->lista_de_tids);
		list_remove_and_destroy_element(TABLA_DE_PAGINAS, obtener_indice_del_proceso(id_proceso),free);
	}else{
		
		
		uint32_t tripulante_logico = indice_de_tripulante(aux->lista_de_tids, id_tripulante);
		uint32_t direccion_logica_inicio = 8 + (21*tripulante_logico);
		uint32_t direccion_logica_fin = 8 + (21*tripulante_logico) + 21;
		uint32_t direccion_logica_tareas;
		uint32_t ultima_direccion_logica = list_size(aux->lista_de_marcos) * TAMANIO_PAGINAS;


		t_list* lista_de_marcos_de_tareas;
		necesito_en_ppal(4, 8, id_proceso);
		lista_de_marcos_de_tareas = obtener_marcos_segun_direccion_logica(4, 8, aux->lista_de_marcos);
		direccion_logica_tareas = modificar_direccion_tareas(lista_de_marcos_de_tareas, (4%TAMANIO_PAGINAS));


		uint32_t lista_tareas = tamanio_lista_tareas(id_proceso, direccion_logica_tareas);
		t_list* lista_marcos_borrado = compactar_tripulante(id_proceso, direccion_logica_inicio, ultima_direccion_logica, lista_tareas);
		uint32_t pagina_inicio_borrado = list_get(lista_marcos_borrado, 0);
		uint32_t pagina_fin_borrado = list_get(lista_marcos_borrado, list_size(lista_marcos_borrado) - 1);
		liberar_marcos(lista_marcos_borrado);
		liberar_tabla(aux->lista_de_marcos, aux->lista_de_presencia, pagina_inicio_borrado, pagina_fin_borrado);
		list_remove(aux->lista_de_tids, tripulante_logico);
		list_destroy(lista_marcos_borrado);
		list_destroy(lista_de_marcos_de_tareas);
	}
}

t_list* deconstruir_string(char* array){
	t_list* aux = list_create();
	uint32_t contador = 0;
	uint32_t contador_palabras = 0;

	while (array[contador]!=NULL){
		if(array[contador]=='-'){
			uint32_t aux2=0;
			char* palabra;
			palabra = (char*) malloc(sizeof(char)*contador_palabras);
			for(uint32_t i = contador-contador_palabras; i <= contador; i++ ){
				palabra[aux2]=array[i];
				aux2++;
			}
			palabra[contador_palabras] = NULL;
			list_add(aux, palabra);
			aux2 = 0;
			contador_palabras= -1;
		}
		
		contador++;
		contador_palabras++;	
	}

	uint32_t aux2=0;
	char* palabra;
	palabra = (char*) malloc(sizeof(char)*contador_palabras);
	for(uint32_t i = contador-contador_palabras; i <= contador; i++ ){
		palabra[aux2]=array[i];
		aux2++;
	}
	palabra[contador_palabras] = NULL;
	list_add(aux, palabra);
	aux2 = 0;
	contador_palabras= -1;
	

	return aux;
}

uint32_t obtener_direccion_logica_tareas(uint32_t id_proceso){
	t_tabla_proceso* aux = buscar_proceso(id_proceso);
	necesito_en_ppal(4, 8, id_proceso);
	t_list* marcos_a_modificar;
	marcos_a_modificar = obtener_marcos_segun_direccion_logica(4, 8, aux->lista_de_marcos);
	uint32_t cantidad_de_marcos = list_size(marcos_a_modificar);
	void* memoria_auxi;
	memoria_auxi = (void*) malloc (cantidad_de_marcos*TAMANIO_PAGINAS);
	for(uint32_t i=0; i<cantidad_de_marcos; i++){
		uint32_t aux = list_get(marcos_a_modificar, i);
		memcpy(memoria_auxi+(i*TAMANIO_PAGINAS), MEMORIA + (aux*TAMANIO_PAGINAS), TAMANIO_PAGINAS);
	}
	uint32_t direccion_logica_tareas;
	memcpy(&direccion_logica_tareas, memoria_auxi + 4%TAMANIO_PAGINAS, 4);
	
	list_destroy(marcos_a_modificar);
	free(memoria_auxi);
	return direccion_logica_tareas;
}

char* obtener_lista_de_tareas(uint32_t direccion_logica, uint32_t id_proceso){
	t_tabla_proceso* aux = buscar_proceso(id_proceso);
	uint32_t cantidad_marcos_proceso = list_size(aux->lista_de_marcos);
	uint32_t cantidad_de_bytes = cantidad_marcos_proceso * TAMANIO_PAGINAS;
	necesito_en_ppal(direccion_logica, cantidad_de_bytes, id_proceso);
	t_list* marcos_a_copiar;
	marcos_a_copiar = obtener_marcos_segun_direccion_logica(direccion_logica, cantidad_de_bytes, aux->lista_de_marcos);
	uint32_t cantidad_de_marcos = list_size(marcos_a_copiar);
	

	void* memoria_auxi;
	memoria_auxi = (void*) malloc (cantidad_de_marcos* TAMANIO_PAGINAS);

	for(uint32_t i=0; i<cantidad_de_marcos; i++){
		uint32_t aux = list_get(marcos_a_copiar, i);
		memcpy(memoria_auxi+(i*TAMANIO_PAGINAS), MEMORIA + (aux*TAMANIO_PAGINAS), TAMANIO_PAGINAS);
	}

	uint32_t tamanio_tareas = 0;
	uint32_t aux3 = 1;
	
	for(uint32_t i= direccion_logica%TAMANIO_PAGINAS; i < cantidad_de_marcos* TAMANIO_PAGINAS; i++){
		char p;
		memcpy(&p, memoria_auxi+i, 1);
		if(p==NULL && tamanio_tareas==0){
			
			tamanio_tareas = aux3;
		}
		aux3++;
	}

	char* tareas;
	tareas = (char*) malloc(sizeof(char)*tamanio_tareas);

	for(uint32_t i= direccion_logica%TAMANIO_PAGINAS; i < tamanio_tareas; i++){
		memcpy(tareas+ i - direccion_logica%TAMANIO_PAGINAS, memoria_auxi+i, 1);
	}
	
	free(memoria_auxi);
	list_destroy(marcos_a_copiar);

	return tareas;
}

uint32_t obtener_id_proxima_tarea(uint32_t id_proceso, uint32_t id_tripulante){
	t_tabla_proceso* aux = buscar_proceso(id_proceso);
	uint32_t tripulante_logico = indice_de_tripulante(aux->lista_de_tids, id_tripulante);
	uint32_t direccion_logica_inicio = 8 + 5 + 8 +(21*tripulante_logico);
	uint32_t direccion_logica_fin = 8 + 5 + 8 +(21*tripulante_logico) + 4;

	necesito_en_ppal(direccion_logica_inicio, direccion_logica_fin, id_proceso);
	t_list* marcos_a_copiar;
	marcos_a_copiar = obtener_marcos_segun_direccion_logica(direccion_logica_inicio, direccion_logica_fin, aux->lista_de_marcos);
	uint32_t cantidad_de_marcos = list_size(marcos_a_copiar);
	void* memoria_auxi;
	memoria_auxi = (void*) malloc (cantidad_de_marcos*TAMANIO_PAGINAS);
	for(uint32_t i=0; i<cantidad_de_marcos; i++){
		uint32_t aux = list_get(marcos_a_copiar, i);
		memcpy(memoria_auxi+(i*TAMANIO_PAGINAS), MEMORIA + (aux*TAMANIO_PAGINAS), TAMANIO_PAGINAS);
	}
	uint32_t direccion_proxima_tarea;
	memcpy(&direccion_proxima_tarea, memoria_auxi + direccion_logica_inicio%TAMANIO_PAGINAS, 4);

	direccion_proxima_tarea++;
	memcpy(memoria_auxi +  direccion_logica_inicio%TAMANIO_PAGINAS, &direccion_proxima_tarea, 4);
	setear_memoria(marcos_a_copiar, memoria_auxi);

	list_destroy(marcos_a_copiar);

	return (direccion_proxima_tarea - 1);
};

char* proxima_tarea(uint32_t id_proceso, uint32_t id_tripulante){
	uint32_t direccion_logica_tareas = obtener_direccion_logica_tareas(id_proceso);
	char* todas_las_tareas = obtener_lista_de_tareas(direccion_logica_tareas, id_proceso);
	t_list* lista_de_tareas = deconstruir_string(todas_las_tareas);
	uint32_t proxima_tarea = obtener_id_proxima_tarea(id_proceso, id_tripulante);
	uint32_t cantidad_tareas = list_size(lista_de_tareas);
	free(todas_las_tareas);
	if(proxima_tarea < cantidad_tareas) {
		char* aux = list_get(lista_de_tareas, proxima_tarea);
		void* memoria_auxiliar;
		memoria_auxiliar = (void*) malloc (strlen(aux)+1);
		memcpy(memoria_auxiliar, aux, strlen(aux)+1);
		list_destroy_and_destroy_elements(lista_de_tareas, free);
		return memoria_auxiliar;
	} else {
		return "NULL";
	}
}

void actualizar_estado(uint32_t id_proceso, uint32_t id_tripulante, char nuevo_estado){
	t_tabla_proceso* aux = buscar_proceso(id_proceso);
	uint32_t tripulante_logico = indice_de_tripulante(aux->lista_de_tids, id_tripulante);
	uint32_t direccion_logica_inicio = 8 + 4 +(21*tripulante_logico);
	uint32_t direccion_logica_fin = direccion_logica_inicio + 1;

	necesito_en_ppal(direccion_logica_inicio, direccion_logica_fin, id_proceso);
	t_list* marcos_a_copiar;
	marcos_a_copiar = obtener_marcos_segun_direccion_logica(direccion_logica_inicio, direccion_logica_fin, aux->lista_de_marcos);
	uint32_t cantidad_de_marcos = list_size(marcos_a_copiar);
	void* memoria_auxi;
	memoria_auxi = (void*) malloc (cantidad_de_marcos*TAMANIO_PAGINAS);
	for(uint32_t i=0; i<cantidad_de_marcos; i++){
		uint32_t aux = list_get(marcos_a_copiar, i);
		memcpy(memoria_auxi+(i*TAMANIO_PAGINAS), MEMORIA + (aux*TAMANIO_PAGINAS), TAMANIO_PAGINAS);
	}
	char estado;
	memcpy(&estado, memoria_auxi + direccion_logica_inicio%TAMANIO_PAGINAS, 1);
	estado = nuevo_estado;

	memcpy(memoria_auxi +  direccion_logica_inicio%TAMANIO_PAGINAS, &estado, 1);
	setear_memoria(marcos_a_copiar, memoria_auxi);

	list_destroy(marcos_a_copiar);
}

char* estado_marco(uint32_t marco){
	if(ESTADO_MARCOS[marco]==0){
		return "LIBRE";
	} else {
		return "OCUPADO";
	}
}

void hacer_dump(){
	char *timestamp = (char *)malloc(sizeof(char) * 16);
	char *timestamp2 = (char *)malloc(sizeof(char) * 16);
	time_t ltime;
	ltime=time(NULL);
	struct tm *tm;
	tm=localtime(&ltime);

	sprintf(timestamp,"Dump: %02d/%02d/%04d %02d:%02d:%02d", tm->tm_mday, tm->tm_mon, 
    tm->tm_year+1900, tm->tm_hour, tm->tm_min, tm->tm_sec);

	sprintf(timestamp2,"%02d%02d%04d_%02d%02d%02d", tm->tm_mday, tm->tm_mon, 
    tm->tm_year+1900, tm->tm_hour, tm->tm_min, tm->tm_sec);

	t_list* lista_dump;
	lista_dump = list_create();

	uint32_t aux_pid = 0;
	uint32_t aux_pag = 0;
	

	void barrer_un_proceso(uint32_t marco){
		if(marco<OFFSET){
			dump_memoria dto;
			dto.marco = marco;
			dto.estado = estado_marco(marco);
			dto.proceso =  aux_pid;
			dto.pagina = aux_pag; 

			void* aux;
			aux = malloc(sizeof(dump_memoria));

			memcpy(aux, &dto, sizeof(dump_memoria));

			list_add(lista_dump, aux);
		}
		aux_pag++;
	}

	void llenar_lista(t_tabla_proceso* item_tabla){
		aux_pid = item_tabla->pid;
		list_iterate(item_tabla->lista_de_marcos, barrer_un_proceso);
		aux_pag = 0; 
	}

	bool ordenar(dump_memoria* a, dump_memoria*b){
		return (a->marco < b->marco);
	}

	list_iterate(TABLA_DE_PAGINAS, llenar_lista);
	list_sort(lista_dump, ordenar);	

	char filename[32];
	strcpy(filename, "dumps/");
	strcat(filename, "Dump_<");
	strcat(filename, timestamp2);
	strcat(filename, ">");

    FILE *fp = fopen(filename, "w");
    if (fp == NULL)
    {
        printf("Error opening the file %s", filename);
        return -1;
    }
	fprintf(fp, "%s\n", timestamp);

	void imprimir_en_archivo(dump_memoria* data){
		fprintf(fp, "Marco:%d Estado:%s Proceso:%d Pagina:%d \n",data->marco, data->estado, data->proceso, data->pagina);
	}

	list_iterate(lista_dump, imprimir_en_archivo);

    fclose(fp);

	list_destroy(lista_dump);
}

void mostrar_array_marcos(){
	printf("\n*****ARRAY DE MARCOS*****\n");
	for(uint32_t i = 0; i<CANTIDAD_MARCOS; i++){
		if(i<10)
			printf("--%d ", i);
		if(i>=10 && i<100)
			printf("-%d ", i);	
		if(i>=100 && i<1000)
			printf("%d ", i);
	}
	printf("\n");

	for(uint32_t a = 0; a<CANTIDAD_MARCOS; a++){
		printf("--%d ", ESTADO_MARCOS[a]);
	}
	printf("\n*************************\n\n");
}

void mostrar_array_timestamp(){
	printf("\n*****ARRAY DE TIMESTAMP*****\n");
	for(uint32_t i = 0; i<CANTIDAD_MARCOS; i++){
		if(i<10)
			printf("--%d ", i);
		if(i>=10 && i<100)
			printf("-%d ", i);	
		if(i>=100 && i<1000)
			printf("%d ", i);
	}
	printf("\n");

	for(uint32_t a = 0; a<CANTIDAD_MARCOS; a++){
		printf("-%d ", TIMESTAMP_MARCOS[a]);
	}
	printf("\n*************************\n\n");
}

void mostrar_array_bit_uso(){
	printf("\n*****BIT DE USO*****\n");
	for(uint32_t i = 0; i<CANTIDAD_MARCOS; i++){
		if(i<10)
			printf("--%d ", i);
		if(i>=10 && i<100)
			printf("-%d ", i);	
		if(i>=100 && i<1000)
			printf("%d ", i);
	}
	printf("\n");

	for(uint32_t a = 0; a<CANTIDAD_MARCOS; a++){
		printf("-%d ", ARRAY_BIT_USO[a]);
	}
	printf("\n*************************\n\n");
}

void mostrar_array_marcos_virtuales(){
	printf("\n*****ARRAY DE MARCOS VIRTUALES*****\n");
	for(uint32_t i = 0; i<CANTIDAD_MARCOS_VIRTUALES; i++){
		if(i<10)
			printf("--%d ", i);
		if(i>=10 && i<100)
			printf("-%d ", i);	
		if(i>=100 && i<1000)
			printf("%d ", i);
	}
	printf("\n");

	for(uint32_t a = 0; a<CANTIDAD_MARCOS_VIRTUALES; a++){
		printf("--%d ", ESTADO_MARCOS_VIRTUALES[a]);
	}
	printf("\n*************************\n\n");
}

void mostrar_lista(t_list* lista){
	void printer(uint32_t num){
		printf("%d ", num);
	}
	printf("La lista es: ");
	list_iterate(lista, printer);
	printf("\n");
}

void mostrar_tabla_de_paginas(){
	
	printf("\n*****TABLA DE PAGINAS*****\n");
	void imprimir_un_valor(t_tabla_proceso* item_tlb){
		uint32_t i = 0;
		printf("PID: %d - ", item_tlb->pid);

		printf("TIDS: ", item_tlb->pid);
		void imprimir_marco2(uint32_t num){
			printf("%d ", num);
		}

		list_iterate(item_tlb->lista_de_tids, imprimir_marco2);
		
		printf("- ");
		void imprimir_marco(uint32_t num){
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

void iniciar_patota_en_mapa(uint32_t pid, t_list* lista_tcb){

	void agregar_a_mapa(t_tcb* tcb){
			
			t_mapa dto;
			dto.pid = pid;
			dto.tid = tcb->tid;
			dto.mapid = ID_MAPA;
			ID_MAPA ++;
			
			void* aux;
			aux = malloc(sizeof(t_mapa));

			memcpy(aux, &dto, sizeof(t_mapa));

			list_add(TABLA_DE_MAPA, aux);
			//iniciar
	}

	list_iterate(lista_tcb, agregar_a_mapa);
}

char obtener_id_mapa(uint32_t pid, uint32_t tid){
	t_mapa* aux;
	bool buscar_id_mapa(t_mapa* personaje){
		return (personaje->pid == pid && personaje->tid == tid);
	}
	aux = list_find(TABLA_DE_MAPA, buscar_id_mapa);
	return aux->mapid;
}

void liberador_lista_uints(uint32_t lista_num){
	printf("%d", lista_num);
	
}


uint32_t main () {
	
	CANTIDAD_MARCOS = obtener_tamanio_array_de_marcos();
	ESTADO_MARCOS = (uint32_t *) malloc( sizeof (uint32_t) * CANTIDAD_MARCOS);
	TIMESTAMP_MARCOS = (uint32_t *) malloc( sizeof (uint32_t) * CANTIDAD_MARCOS);
	ARRAY_BIT_USO =  (uint32_t *) malloc( sizeof (uint32_t) * CANTIDAD_MARCOS);
	CANTIDAD_MARCOS_VIRTUALES = obtener_tamanio_array_de_marcos_virtuales();
	ESTADO_MARCOS_VIRTUALES = (uint32_t *) malloc( sizeof (uint32_t) * CANTIDAD_MARCOS_VIRTUALES);
	
	//inicializo en 0 todo el bitmap

	for (uint32_t i = 0; i < CANTIDAD_MARCOS; i++) {
		ESTADO_MARCOS[i] = 0;
	}

	for (uint32_t i = 0; i < CANTIDAD_MARCOS; i++) {
		TIMESTAMP_MARCOS[i] = 0;
	}

	for (uint32_t i = 0; i < CANTIDAD_MARCOS; i++) {
		ARRAY_BIT_USO[i] = 0;
	}

	for (uint32_t i = 0; i < CANTIDAD_MARCOS_VIRTUALES; i++) {
		ESTADO_MARCOS_VIRTUALES[i] = 0;
	}
	

	MEMORIA = (void*) malloc (TAMANIO_MEMORIA);
	TABLA_DE_PAGINAS = list_create();


	uint32_t fd = open("cfg/virtualmemory.txt",O_RDWR , S_IRUSR | S_IWUSR);
	struct stat sb;

	ftruncate(fd, TAMANIO_MEMORIA_VIRTUAL);

	if(fstat(fd, &sb) == -1) {
		perror("No pude obtener el tamaño del archivo.\n");
	}

	printf("Tamaño del Archivo: %ld\n",sb.st_size);

	MEMORIA_VIRTUAL = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	TABLA_DE_MAPA = list_create();

	t_pcb mockpcb;
	mockpcb.pid= 1;
	mockpcb.tareas= 50;

	t_tcb* mocktcb = malloc(sizeof(t_tcb));
	mocktcb->estado= 'A';
	mocktcb->posicion_x=2;
	mocktcb->posicion_y=3;
	mocktcb->proxima_instruccion=0;
	mocktcb->puntero_pcb= 0;
	mocktcb->tid= 10;

	t_tcb* mocktcb4 = malloc(sizeof(t_tcb));
	mocktcb4->estado= 'A';
	mocktcb4->posicion_x=2;
	mocktcb4->posicion_y=3;
	mocktcb4->proxima_instruccion=0;
	mocktcb4->puntero_pcb= 0;
	mocktcb4->tid= 11;

	t_list* tcblist = list_create();
	list_add(tcblist, mocktcb);
	
	char* pepe;
	pepe = "A00;1;1;5-A01;1;1;5-A02;1;1;5";

	estructura_administrativa_paginacion mockwrapeado;
	mockwrapeado.pcb = mockpcb;
	mockwrapeado.lista_de_tcb = tcblist;
	mockwrapeado.lista_de_tareas = pepe;

	t_pcb mockpcb2;
	mockpcb2.pid= 2;
	mockpcb2.tareas= 10;

	t_tcb* mocktcb2 = malloc(sizeof(t_tcb));
	mocktcb2->estado= 1;
	mocktcb2->posicion_x=2;
	mocktcb2->posicion_y=3;
	mocktcb2->proxima_instruccion=0;
	mocktcb2->puntero_pcb= 0;
	mocktcb2->tid= 20;

	t_list* tcblist2 = list_create();
	list_add(tcblist2, mocktcb2);
	
	char* pepe2;
	pepe2 = "B00;3;3;5-B01;3;3;5-B02;3;3;5";

	estructura_administrativa_paginacion mockwrapeado2;
	mockwrapeado2.pcb = mockpcb2;
	mockwrapeado2.lista_de_tcb = tcblist2;
	mockwrapeado2.lista_de_tareas = pepe2;

	t_pcb mockpcb3;
	mockpcb3.pid= 3;
	mockpcb3.tareas= 10;

	t_tcb* mocktcb3 = malloc(sizeof(t_tcb));
	mocktcb3->estado= 1;
	mocktcb3->posicion_x=2;
	mocktcb3->posicion_y=3;
	mocktcb3->proxima_instruccion=0;
	mocktcb3->puntero_pcb= 0;
	mocktcb3->tid= 30;

	t_list* tcblist3 = list_create();
	list_add(tcblist3, mocktcb3);
	
	char* pepe3;
	pepe3 = "C00;5;5;5-C01;5;5;5-C02;5;5;5-C03;5;5;5-C04;5;5;5-C05;5;5;5-C06;5;5;5-C07;5;5;5-C08;5;5;5";

	estructura_administrativa_paginacion mockwrapeado3;
	mockwrapeado3.pcb = mockpcb3;
	mockwrapeado3.lista_de_tcb = tcblist3;
	mockwrapeado3.lista_de_tareas = pepe3;



	


	iniciar_patota(&mockwrapeado);
	mostrar_tabla_de_paginas();
	iniciar_patota(&mockwrapeado2);
	mostrar_tabla_de_paginas();
	iniciar_patota(&mockwrapeado3);
	mostrar_tabla_de_paginas();
	mostrar_array_bit_uso();	
	
	cambiar_ubicacion_tripulante(1, 10, 2, 3);
	mostrar_tabla_de_paginas();
	cambiar_ubicacion_tripulante(2, 20, 3, 4);
	mostrar_tabla_de_paginas();
	cambiar_ubicacion_tripulante(3, 30, 4, 5);
	mostrar_tabla_de_paginas();
	cambiar_ubicacion_tripulante(1, 10, 2, 3);
	mostrar_tabla_de_paginas();
	cambiar_ubicacion_tripulante(2, 20, 3, 4);
	mostrar_tabla_de_paginas();
	cambiar_ubicacion_tripulante(3, 30, 4, 5);
	mostrar_tabla_de_paginas();
	cambiar_ubicacion_tripulante(1, 10, 2, 3);
	mostrar_tabla_de_paginas();
	expulsar_tripulante(1, 10);
	mostrar_tabla_de_paginas();
	cambiar_ubicacion_tripulante(2, 20, 3, 4);
	mostrar_tabla_de_paginas();
	expulsar_tripulante(2, 20);
	mostrar_tabla_de_paginas();
	cambiar_ubicacion_tripulante(3, 30, 4, 5);
	mostrar_tabla_de_paginas();
	cambiar_ubicacion_tripulante(3, 30, 4, 5);



	/*
	mostrar_array_marcos();
	mostrar_tabla_de_paginas();
	mostrar_array_marcos_virtuales();


	memcpy(&b, MEMORIA+35, 4);
	printf("El valor es %d\n",redondear_para_arriba(11,2));*/
	
	//expulsar_tripulante(4, 2);
	/*t_list* loro = list_create();
	list_add(loro, 15002);
	list_add(loro, 15003);

	uint32_t b;
	memcpy(&b, MEMORIA+30, 4);
	printf("El valor es %d\n",b);

	alojar(loro);*/

	//memcpy(&b, MEMORIA+30, 4);
	//printf("El valor es %d\n",b);

	

	/*mostrar_array_marcos();
	mostrar_tabla_de_paginas();
	mostrar_array_marcos_virtuales();*/

	/*char* pedepepe;
	pedepepe = (char*) malloc(sizeof(char)*18);
	pedepepe = "Alla-en-la-fuente";

	t_list* loco;
	loco = deconstruir_string(pedepepe);
	printf("%s", list_get(loco, 3));*/
	/*proxima_tarea(2, 19);
	proxima_tarea(2, 19);
	char* pepepepe = proxima_tarea(2, 19);
	printf("EL VALOR QUE SALIO ES %s", pepepepe);*/

	/*actualizar_estado(2, 19, 'B');

	mostrar_array_marcos();
	mostrar_tabla_de_paginas();
	mostrar_array_marcos_virtuales();*/
	/*char b;
	memcpy(&b, MEMORIA+30, 4);
	printf("El valor es %c\n",b);*/


	/*mostrar_array_marcos();
	mostrar_tabla_de_paginas();
	mostrar_array_marcos_virtuales();

	uint32_t v;
	memcpy(&v, MEMORIA+35, 4);
	printf("Virtual %d\n",v);*/
	/*char axs;

	iniciar_patota_en_mapa(2, tcblist);
	axs = obtener_id_mapa(2, 3);
	printf("%c\n", axs);*/

	//printf("%d\n", obtener_indice_del_proceso(4));

	free(mocktcb);
	free(mocktcb2);
	free(mocktcb3);
	free(mocktcb4);

	list_destroy(tcblist);
	list_destroy(tcblist2);
	list_destroy(tcblist3);

	return 0;
}