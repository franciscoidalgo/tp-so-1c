#include "instruccion.h"
#include "fs_functions.h"
#include "superbloque.h"

#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/bitarray.h>
#include <commons/collections/list.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <inttypes.h>
#include <errno.h>
#include <signal.h>
#include <dirent.h>
#include <stdio.h>
#include <ctype.h>


#define FORMATO_RECURSO "SIZE=0\nBLOCK_COUNT=0\nBLOCKS=[]\nCARACTER_LLENADO=\nMD5_ARCHIVO=\n"
#define FORMATO_BITACORA "SIZE=0\nBLOCKS=[]\n"
#define TRUNCATE_ERROR -1
#define LOGGEAR_CAMBIOS 'y'
#define NO_LOGGEAR 'n'


char* path;
char* path_files;
char* path_bitacoras;
char* path_sb;
char* path_blocks;

t_config* config;
t_log* logger;
char* blocks_p;
pthread_t hilo_blocks;
pthread_mutex_t mutex_blocks;
uint32_t blocks;
uint32_t block_size;

//declaracion de funciones locales usadas en inicializacion
void recuperar_fs(int);
void iniciar_en_limpio();

void generar_directorios(char*);
void generar_superbloque();
void generar_blocks();
char* generar_md5(char*, size_t);
char* get_blocks_data (t_config* recurso);
void chequear_blocks_data(t_config* recurso, char log_option);
void chequear_block_count(t_config* recurso, char log_option);
void generar_bitacora(uint32_t tripulante_id, char* entrada, int size_entrada);
void generar_recurso(char* nombre_recurso, int cantidad);
void consumir_recurso(char* nombre_recurso, int cantidad);
void interpretar_mensaje_discordiador (char* mensaje);
void formatear_meta_file (int, char*);
void generar_meta_recursos();
char* obtener_bitacora (uint32_t);

//Hash Table para recursos y sus caracteres de llenado
t_instruccion tabla_comandos []={
	{"GENERAR_OXIGENO", "Oxigeno", 'O', generar_recurso},
	{"GENERAR_COMIDA", "Comida", 'C', generar_recurso},
	{"GENERAR_BASURA", "Basura", 'B', generar_recurso},
	{"CONSUMIR_OXIGENO", "Oxigeno", 'O', consumir_recurso},
	{"CONSUMIR_COMIDA", "Comida", 'C', consumir_recurso},
	{"DESCARTAR_BASURA", "Basura", 'B', consumir_recurso},
	{"\0", "\0", 0, NULL}
};

//Funcion que genera las estructuras necesarias
void iniciar_en_limpio(t_config* config_fs, t_log* logger_fs){
    pthread_mutex_init(&mutex_blocks, NULL);
    logger = logger_fs;
    config = config_fs;
	path = config_get_string_value(config, "PUNTO_MONTAJE");
	blocks = config_get_int_value(config, "BLOCKS");
	block_size = config_get_int_value(config, "BLOCKS_SIZE");

	generar_directorios(path);
	generar_superbloque();
	generar_blocks();
	generar_meta_recursos();
	log_info(logger, "El sistema se ha inicializado correctamente!");	

}


void generar_meta (char* nombre_recurso, char caracter_de_llenado){
	char* file_name = string_from_format("%s.ims", nombre_recurso);
	char* file_path = string_duplicate(path_files);
	int file_size;

	string_append(&file_path, "/");
	string_append(&file_path, file_name);
	int fd = open(file_path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	file_size = lseek(fd, 0, SEEK_END);
	if(file_size <= 1){
		formatear_meta_file(fd, FORMATO_RECURSO);
		close(fd);
		t_config* recurso = config_create(file_path);
		char caracter_de_llenado_array[2];
		caracter_de_llenado_array[0] = caracter_de_llenado;
		caracter_de_llenado_array[1] = '\0';
		config_set_value(recurso, "CARACTER_LLENADO", caracter_de_llenado_array);
		config_save(recurso);
		config_destroy(recurso);
	}
	close(fd);
	
	free(file_name);
	free(file_path);
}


void generar_meta_recursos (){
	for(int i = 0; tabla_comandos[i].funcion != NULL; i++){
		generar_meta(tabla_comandos[i].nombre_recurso, tabla_comandos[i].caracter_de_llenado);
	}
}


void crear_directorio(char* path){
	int chequear_error;

	chequear_error = mkdir(path, 0777);

	if(!chequear_error){
		log_info(logger, "Se creo el directorio: %s.", path);
	}else if(errno != EEXIST){
		perror("Error al generar un directorio.");
	}
}


void generar_directorios(char* path_base){
	path_files = string_duplicate(path_base);
	string_append(&path_files, "/Files");
	path_bitacoras = string_duplicate(path_files);
	string_append(&path_bitacoras, "/Bitacoras");
	crear_directorio(path_base);
	crear_directorio(path_files);
	crear_directorio(path_bitacoras);
}


void generar_superbloque(){
	int sb_file;
	
	path_sb = string_duplicate(path);
	string_append(&path_sb, "/SuperBloque.ims");

	sb_file = open(path_sb, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	if(lseek(sb_file, 0, SEEK_END) > 1){
		log_info(logger, "Se recupero un SuperBloque existente.");
		close(sb_file);
		return;
	}

	void* superbloque;

	uint32_t blocks_en_bytes = blocks_to_bytes(blocks);

	char* puntero_a_bits = calloc(blocks_en_bytes, sizeof(char));

	size_t tamanio_sb = len(blocks);

	//Setea su tamaño para poder mapearlo
	if(ftruncate(sb_file, tamanio_sb) == TRUNCATE_ERROR){
		perror("Error al generar archivo SuperBloque.ims .");
	}
	fsync(sb_file);

	superbloque = mmap (NULL, tamanio_sb, PROT_WRITE | PROT_READ, MAP_SHARED, sb_file, 0);
	int offset = 0;
	//Copia los datos necesarios al superbloque
	memcpy(superbloque+offset, &blocks, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(superbloque+offset, &block_size, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(superbloque+offset, puntero_a_bits, blocks_en_bytes);

	munmap(superbloque, tamanio_sb);

	//Una vez creado el archivo, se libera la memoria
	close(sb_file);
	free(puntero_a_bits);
	log_info(logger, "Se genero el SuperBloque.");

}


void actualizar_blocks(){
	int sync_time = config_get_int_value(config, "TIEMPO_SINCRONIZACION");
	while(1){
		pthread_mutex_lock(&mutex_blocks);
		msync(blocks_p, blocks * block_size, MS_SYNC);
		pthread_mutex_unlock(&mutex_blocks);
		log_info(logger, "Se actualizaron los bloques en el fs.");
		sleep(sync_time);
	}
}


void generar_blocks(){
	int b_file;
	
	path_blocks = string_duplicate(path);
	string_append(&path_blocks, "/Blocks.ims");

	b_file = open(path_blocks, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

	if(lseek(b_file, 0, SEEK_END) > 1){
		log_info(logger, "Se recupero un Blocks existente.");
	}else{
		log_info(logger, "Se genero el archivo Blocks.");
		if(ftruncate(b_file, blocks * block_size)==TRUNCATE_ERROR){
			perror("Error al generar la metadata de un archivo.");
		}
		fsync(b_file);
	}

	blocks_p = (char*) mmap(NULL, blocks * block_size, PROT_WRITE | PROT_READ, MAP_SHARED, b_file, 0);
	close(b_file);

	pthread_create(&hilo_blocks, NULL, (void*) actualizar_blocks, NULL);
	pthread_detach(hilo_blocks);

}


void formatear_meta_file (int fd, char* formato){
	char* file_content;

	if(ftruncate(fd, strlen(formato))==TRUNCATE_ERROR){
		perror("Error al generar la metadata de un archivo.");
	}
	fsync(fd);
	file_content = mmap(NULL, strlen(formato), PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
	strcpy(file_content, formato);
	munmap(file_content, strlen(formato));
}


int encontrar_block_libre(){
	int numero_bloque;
	for(numero_bloque = 0; blocks_p[numero_bloque*block_size] != 0 && numero_bloque < blocks; numero_bloque++);
	return numero_bloque;
}


void refrescar_bloques (t_list* bloques, t_config* recurso){
	char new_blocks_data [255]; 
	int cant_bloques = list_size(bloques);
	if(list_is_empty(bloques)){
		config_set_value(recurso, "BLOCKS", "[]");
		return;
	}
	int current_index = sprintf(new_blocks_data, "[%s", (char*) list_get(bloques, 0));
	for(int current = 1; current < cant_bloques; current++)
		current_index += sprintf(new_blocks_data + current_index, ",%s", (char*) list_get(bloques, current));
	sprintf(new_blocks_data + current_index, "]");
	config_set_value(recurso, "BLOCKS", new_blocks_data);
}


void actualizar_bitmap (t_list* bloques_en_uso){
	int superbloque_fd = open(path_sb, O_RDWR, S_IRUSR | S_IWUSR);
	int superbloque_file_size = lseek(superbloque_fd, 0, SEEK_END) + 1;
	void* superbloque_mapped = mmap(NULL, superbloque_file_size, PROT_WRITE | PROT_READ, MAP_SHARED, superbloque_fd, 0);
	int block_list_size = list_size(bloques_en_uso);
	uint32_t blocks;
	int sizeofbitmap;
	char* bitmap;
	int offset_bitmap = 2 * sizeof(uint32_t);
	t_bitarray* bitarray;
	

	memcpy(&blocks, superbloque_mapped, sizeof(uint32_t));
	sizeofbitmap = blocks_to_bytes(blocks);
	bitmap = malloc(sizeofbitmap);
	memcpy(bitmap, superbloque_mapped + offset_bitmap, sizeofbitmap);
	bitarray = bitarray_create_with_mode(bitmap, sizeofbitmap, LSB_FIRST);

	for(int i=0; i< block_list_size; i++){
		bitarray_set_bit(bitarray, atoi(list_get(bloques_en_uso, i)));
	}

	memcpy(superbloque_mapped + offset_bitmap, bitarray->bitarray, sizeofbitmap);

	munmap(superbloque_mapped, superbloque_file_size);
	free(bitmap);
	bitarray_destroy(bitarray);

	
}


void limpiar_lista_bloques(t_list* lista_bloques){
	t_link_element* elemento_actual;
	elemento_actual = lista_bloques->head;
	for (int i=0; i < list_size(lista_bloques);i++){
		free(elemento_actual->data);
		elemento_actual = elemento_actual->next;
	}
}


void interpretar_mensaje_discordiador (char* mensaje){
	t_instruccion* instruccion;
	char comando [255];
	int cantidad;
	sscanf(mensaje, "%s %d", comando, &cantidad);
	instruccion = get_instruccion(comando, tabla_comandos);
	ejecutar_instruccion(instruccion, cantidad);
}


void liberar_blocks_array(char** blocks_array){
	for(int i = 0; blocks_array[i] != NULL; i++){
		free(blocks_array[i]);
	}
}


void generar_file(t_config* recurso, char* entrada, int size_entrada){
	int current_block;
	int bytes_escritos = 0;
	int bytes_a_escribir;

	char**  blocks_array = config_get_array_value(recurso, "BLOCKS");
	int size;
	char* size_string;
	
	size = config_get_int_value(recurso, "SIZE");

	t_list* lista_bloques = list_create();


	if(blocks_array[0] != NULL){
		list_add(lista_bloques, blocks_array[0]);
		int offset = 0;
		int index_ultimo_bloque;
		for (index_ultimo_bloque = 1; blocks_array[index_ultimo_bloque] != NULL; index_ultimo_bloque++){
			list_add(lista_bloques, blocks_array[index_ultimo_bloque]);
		}
		index_ultimo_bloque--;
		int ultimo_bloque = atoi(blocks_array[index_ultimo_bloque]);
		for(offset = 0; offset < block_size && blocks_p[ultimo_bloque * block_size + offset] != 0; offset++);
		if(offset != block_size){
			if(size_entrada < block_size - offset){
				memcpy((void*) blocks_p + ultimo_bloque * block_size + offset, entrada, size_entrada);
				bytes_escritos = size_entrada;
			}else{
				memcpy((void*) blocks_p + ultimo_bloque * block_size + offset, entrada, block_size - offset);
				bytes_escritos = block_size - offset;
			}
		}
		
	}

	while(bytes_escritos < size_entrada){
		current_block = encontrar_block_libre();
		if(size_entrada - bytes_escritos > block_size){
			bytes_a_escribir = block_size;
		}else{
			bytes_a_escribir = size_entrada - bytes_escritos;
		}
		memcpy((void*) blocks_p + current_block * block_size, (void*) entrada + bytes_escritos, bytes_a_escribir);
		bytes_escritos += bytes_a_escribir;
		list_add(lista_bloques, string_from_format("%d", current_block));
	}
	size += size_entrada;
	size_string = string_from_format("%d", size);

	config_set_value(recurso, "SIZE", size_string);
	refrescar_bloques(lista_bloques, recurso);
	config_save(recurso);
	actualizar_bitmap(lista_bloques);	
	
	limpiar_lista_bloques(lista_bloques);
	list_destroy(lista_bloques);
	free(size_string);
	free(blocks_array);

}


char* generar_path_bitacora(uint32_t tripulante_id){
	char* file_name = string_from_format("Tripulante%lu.ims", tripulante_id);
	char* file_path = string_duplicate(path_bitacoras);
	string_append(&file_path, "/");
	string_append(&file_path, file_name);
	free(file_name);
	return file_path;
}


char* obtener_bitacora (uint32_t tripulante_id){
	t_config* bitacora;
	char* file_path = generar_path_bitacora(tripulante_id);
	char* string_bitacora;
	int size;
	char** blocks_array;
	int cant_bloques;
	int bytes_leidos = 0;
	int block_index = 0;

	if(access(file_path, F_OK) != 0){
		log_info(logger, "Se solicito una bitacora inexistente.");
		return NULL;
	}
	
	bitacora= config_create(file_path);
	size= config_get_int_value(bitacora, "SIZE");
	blocks_array = config_get_array_value(bitacora, "BLOCKS");
	string_bitacora = malloc(size+1);
	
	for(cant_bloques = 0; blocks_array[cant_bloques] != NULL; cant_bloques++);

	while(bytes_leidos < size && block_index < cant_bloques){
		if(size - bytes_leidos >= block_size){
			memcpy(
				string_bitacora + bytes_leidos, 
				blocks_p + block_size * atoi(blocks_array[block_index]), 
				block_size
				);
			bytes_leidos += block_size;
		}else{
			memcpy(
				string_bitacora + bytes_leidos,
				blocks_p + block_size * atoi(blocks_array[block_index]), 
				size - bytes_leidos
				);
			bytes_leidos+= size-bytes_leidos;
		}
		free(blocks_array[block_index]);
		block_index++;
	}
	string_bitacora[size] = '\0';
	
	config_destroy(bitacora);
	free(blocks_array);
	free(file_path);
	return string_bitacora;
}


void generar_bitacora(uint32_t tripulante_id, char* entrada, int sizeofentrada){
	t_config* recurso;
	char* file_path = generar_path_bitacora(tripulante_id);
	int fd = open(file_path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	if(lseek(fd, 0, SEEK_END) <= 1){
		formatear_meta_file(fd, FORMATO_BITACORA);
	}
	close(fd);
	recurso = config_create(file_path);
	pthread_mutex_lock(&mutex_blocks);
	generar_file(recurso, entrada, sizeofentrada);
	pthread_mutex_unlock(&mutex_blocks);
	free(file_path);
	config_destroy(recurso);

}


void consumir_recurso_en_blocks (t_config* recurso, int cantidad){
	int current_block;
	int cantidad_restante = cantidad;
	int current_index;
	int offset_primer_bloque_libre;

	char** blocks_array;
	
	int size;
	char* size_string;
	
	blocks_array = config_get_array_value(recurso, "BLOCKS");
	size = config_get_int_value(recurso, "SIZE");
	
	if(blocks_array[0] == NULL){
		log_info(logger, "Se quiso consumir un recurso que todavia no se creo.");
		return;
	}
	
	t_list* lista_bloques = list_create();
	for(int i = 0; blocks_array[i] != NULL; i++){
		list_add(lista_bloques, blocks_array[i]);
	}
	current_index = list_size(lista_bloques) - 1;
	current_block = atoi(list_get(lista_bloques, current_index));

	for(offset_primer_bloque_libre = 0; 
	offset_primer_bloque_libre < block_size && blocks_p[current_block * block_size + offset_primer_bloque_libre] != 0; 
	offset_primer_bloque_libre++);
	if(cantidad_restante < offset_primer_bloque_libre){
		memset(blocks_p + current_block * block_size + offset_primer_bloque_libre - cantidad_restante, 0, block_size - offset_primer_bloque_libre + cantidad_restante);
		cantidad_restante = 0;
	}else{
		memset(blocks_p + current_block * block_size, 0, block_size);
		cantidad_restante -= offset_primer_bloque_libre;
		list_remove(lista_bloques, current_index);
	}

	while(cantidad_restante > 0 && current_index > 0){
		current_index--;
		current_block = atoi(list_get(lista_bloques, current_index));
		if(cantidad_restante >= block_size){
			memset(blocks_p + current_block * block_size, 0, block_size);
			cantidad_restante -= block_size;
		}else{
			memset(blocks_p + current_block * block_size + block_size - cantidad_restante, 0, cantidad_restante);
			cantidad_restante = 0;
		}
	}
	if(current_index<0){
		log_info(logger, "Se quiso consumir mas recursos de los disponibles.");
	}
	size -= cantidad + cantidad_restante;
	size_string = string_from_format("%d", size);
	config_set_value(recurso, "SIZE", size_string);
	refrescar_bloques(lista_bloques, recurso);
	config_save(recurso);
	chequear_block_count(recurso, NO_LOGGEAR);
	actualizar_bitmap(lista_bloques);	
	

	list_destroy(lista_bloques);
	liberar_blocks_array(blocks_array);
	free(blocks_array);
	free(size_string);
}


void consumir_recurso (char* nombre_recurso, int cantidad){
	t_config* recurso;
	char* file_name = string_from_format("%s.ims", nombre_recurso);
	char* file_path = string_duplicate(path_files);
	
	string_append(&file_path, "/");
	string_append(&file_path, file_name);
	
	recurso = config_create(file_path);

	log_info(logger, "Se consumiran %d de %s", cantidad, nombre_recurso);
	pthread_mutex_lock(&mutex_blocks);
	consumir_recurso_en_blocks(recurso, cantidad);
	chequear_block_count(recurso, NO_LOGGEAR);
	if(config_get_int_value(recurso, "SIZE") > 0)
		chequear_blocks_data(recurso, NO_LOGGEAR);
	else{
		config_set_value(recurso, "MD5_ARCHIVO", "");
		config_save(recurso);
	}
	pthread_mutex_unlock(&mutex_blocks);
	
	config_destroy(recurso);
	free(file_name);
	free(file_path);

}


void generar_recurso (char* nombre_recurso, int cantidad){
	t_config* recurso;
	char* entrada;
	char* file_name = string_from_format("%s.ims", nombre_recurso);
	char* file_path = string_duplicate(path_files);
	char* caracter_de_llenado_array;

	string_append(&file_path, "/");
	string_append(&file_path, file_name);
	
	recurso = config_create(file_path);

	caracter_de_llenado_array = config_get_string_value(recurso, "CARACTER_LLENADO");

	entrada = string_repeat(caracter_de_llenado_array[0], cantidad);
	pthread_mutex_lock(&mutex_blocks);
	generar_file(recurso, entrada, cantidad);
	chequear_block_count(recurso, NO_LOGGEAR);
	chequear_blocks_data(recurso, NO_LOGGEAR);
	pthread_mutex_unlock(&mutex_blocks);
	log_info(logger, "Se genero %d de %s", cantidad, nombre_recurso);
	config_destroy(recurso);
	free(entrada);
	free(file_name);
	free(file_path);

}


void buscar_recursos_en_path(char* path_elegido, t_list* lista_bloques){
	int i = 0;
	t_config* recurso;
	char** valores;
	char* path_recurso;
	struct dirent *d;
	DIR *dh = opendir(path_elegido);
	while((d = readdir(dh)) != NULL){
		if(string_contains(d->d_name, ".ims")){
			path_recurso = string_duplicate(path_elegido);
			string_append(&path_recurso, "/");
			string_append(&path_recurso, d->d_name);
			recurso = config_create(path_recurso);
			valores = config_get_array_value(recurso, "BLOCKS");
			while(valores[i] != NULL){
				list_add(lista_bloques, valores[i]);
				i++;
			}
			i = 0;
			free(path_recurso);
			free(valores);
			config_destroy(recurso);
		}
	}
	closedir(dh);
}


void chequear_bitmap(t_bitarray* bitmap, t_list* bloques_en_uso){
	int tamanio_lista = list_size(bloques_en_uso);
	int aux;
	memset(bitmap->bitarray, 0, bitmap->size);
	for(int i = 0; i<tamanio_lista; i++){
		aux = atoi(list_get(bloques_en_uso, i));
		bitarray_set_bit(bitmap, aux);
	}
	log_info(logger, "Se chequeo el bitmap.");
}


void chequear_recursos (void* superbloque, int superbloque_file_size){
	t_list* lista_bloques = list_create();
	t_bitarray* bitmap;
	uint32_t blocks;
	char* bitarray;
	int sizeofbitmap;
	int offset_bitmap = sizeof(uint32_t) * 2;

	memcpy(&blocks, superbloque, sizeof(uint32_t));
	sizeofbitmap = blocks_to_bytes(blocks);
	bitarray = malloc(sizeofbitmap);
	memcpy(bitarray, superbloque + offset_bitmap, sizeofbitmap);

	bitmap = bitarray_create_with_mode(bitarray, sizeofbitmap, LSB_FIRST);


	buscar_recursos_en_path(path_files, lista_bloques);
	buscar_recursos_en_path(path_bitacoras, lista_bloques);


	chequear_bitmap(bitmap, lista_bloques);
	memcpy(superbloque + offset_bitmap, bitmap->bitarray, sizeofbitmap);


	bitarray_destroy(bitmap);
	free(bitarray);
	limpiar_lista_bloques(lista_bloques);
	list_destroy(lista_bloques);
}


void chequear_cant_blocks(void* superbloque, int superbloque_file_size,int blocks_file_size){
	
	int cant_real_bloques;
	uint32_t cant_supuesta_bloques;
	uint32_t block_size_calculado;
	
	int offset = 0;
	memcpy(&cant_supuesta_bloques, superbloque, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&block_size_calculado, superbloque + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	
	cant_real_bloques = blocks_file_size / block_size_calculado;

	if(cant_real_bloques != (cant_supuesta_bloques)){
		memcpy(superbloque, &cant_real_bloques, sizeof(uint32_t));
		msync(superbloque, superbloque_file_size, MS_SYNC);
		log_info(logger, "Hubo un sabotaje en la cantidad de bloques pero ya se ha corregido.");
	}

}


void chequear_superbloque(){
	int file_sb;
	int file_bloques;
	void* superbloque;
	
	int superbloque_file_size;
	int blocks_file_size;

	file_sb = open(path_sb, O_RDWR, S_IRUSR | S_IWUSR);
	file_bloques = open(path_blocks, O_RDWR, S_IRUSR | S_IWUSR);

	//Tamanio del Blocks y Superbloque
	superbloque_file_size = lseek(file_sb, 0, SEEK_END) + 1;
	blocks_file_size = lseek(file_bloques, 0, SEEK_END) + 1;

	//Cargo los datos del superbloque
	superbloque = mmap(NULL, superbloque_file_size, PROT_WRITE | PROT_READ, MAP_SHARED, file_sb, 0);
	
	//Chequear sabotaje en cantidad de bloques
	chequear_cant_blocks(superbloque, superbloque_file_size, blocks_file_size);

	//Chequear sabotaje en bitmap
	chequear_recursos(superbloque, superbloque_file_size);


	munmap(superbloque, superbloque_file_size);
	close(file_sb);
	close(file_bloques);


}


void chequear_block_count(t_config* recurso, char log_option){
	int len_blocks;
	char** blocks_array = config_get_array_value(recurso, "BLOCKS");
	char* int_as_array;
	for(len_blocks=0; blocks_array[len_blocks] != NULL; len_blocks++);
	if(config_get_int_value(recurso, "BLOCK_COUNT") != len_blocks){
		int_as_array = string_from_format("%d", len_blocks);
		config_set_value(recurso, "BLOCK_COUNT", int_as_array);
		config_save(recurso);
		free(int_as_array);
		if(log_option == LOGGEAR_CAMBIOS)
			log_info(logger, "Se corrigio un sabotaje en la cantidad de bloques del archivo: %s", recurso->path);
	}
	liberar_blocks_array(blocks_array);
	free(blocks_array);
}


void chequear_file_size(t_config* recurso){
	char** blocks_array = config_get_array_value(recurso, "BLOCKS");
	int size_supuesto = config_get_int_value(recurso, "SIZE");
	int offset;
	int current_block;
	int size = 0;
	char* size_as_array;
	for(int i = 0; blocks_array[i] != NULL; i++){
		current_block = atoi(blocks_array[i]);
		for(offset = 0; offset<block_size && blocks_p[current_block * block_size + offset] != 0; offset++){
			size++;
		}
		free(blocks_array[i]);
	}
	if(size_supuesto != size){
		size_as_array = string_from_format("%d", size);
		config_set_value(recurso, "SIZE", size_as_array);
		config_save(recurso);
		free(size_as_array);
		log_info(logger, "Se corrigio un sabotaje en el tamanio del archivo: %s", recurso->path);
	}
	free(blocks_array);
}


char* get_blocks_data (t_config* recurso){
	int file_size = config_get_int_value(recurso, "SIZE");
	char** blocks_array = config_get_array_value(recurso, "BLOCKS");
	char* blocks_data = calloc(file_size, sizeof(char));
	int current_block;
	int offset;
	int bytes_chequeados = 0;
	for (int i= 0; blocks_array[i] != NULL; i++){
		current_block = atoi(blocks_array[i]);
		for (offset=0; offset < block_size && bytes_chequeados < file_size; offset++){
			memcpy((void*) blocks_data + bytes_chequeados, (void*) blocks_p + current_block * block_size + offset, sizeof(char));
			bytes_chequeados++;
		}
		free(blocks_array[i]);
	}
	free(blocks_array);
	return blocks_data;
}


void restaurar_archivo(t_config* recurso){
	char** blocks_array = config_get_array_value(recurso, "BLOCKS");
	int file_size = config_get_int_value(recurso, "SIZE");
	char* aux = config_get_string_value(recurso, "CARACTER_LLENADO");
	char caracter_de_llenado = aux[0];
	int offset;
	int i;
	int bytes_escritos = 0;
	int current_block;
	//Primero limpio los bloques
	for (i = 0; blocks_array[i] != NULL; i++){
		offset = block_size * atoi(blocks_array[i]);
		memset(blocks_p + offset, 0, block_size);
	}
	//Luego los escribo hasta completar el size
	for(i = 0; blocks_array[i] != NULL; i++){
		current_block = atoi(blocks_array[i]);
		for(offset = 0; offset < block_size && bytes_escritos < file_size; offset++){
			blocks_p [current_block * block_size + offset] = caracter_de_llenado;
			bytes_escritos++;
		}
		free(blocks_array[i]);
	}
	free(blocks_array);
}


void chequear_blocks_data(t_config* recurso, char log_option){
	char* blocks_data = get_blocks_data(recurso);
	int size_recurso = config_get_int_value(recurso, "SIZE");
	char* md5_data = generar_md5 (blocks_data, size_recurso);
	char* md5_archivo = config_get_string_value(recurso, "MD5_ARCHIVO");
	if(strcmp(md5_data, md5_archivo) != 0){
		restaurar_archivo(recurso);
		char* new_blocks_data = get_blocks_data(recurso);
		char* new_md5 = generar_md5(new_blocks_data, config_get_int_value(recurso, "SIZE"));
		config_set_value(recurso, "MD5_ARCHIVO", new_md5);
		config_save(recurso);
		if(log_option == LOGGEAR_CAMBIOS)
			log_info(logger, "La data del archivo %s no coincidia con el contenido del archivo Blocks: se restauro el archivo.", recurso->path);
		free(new_blocks_data);
		free(new_md5);
	}
	free(md5_data);
	free(blocks_data);
}


void chequear_files(char* path_elegido){
	t_config* recurso;
	char* path_recurso;
	struct dirent *d;
	DIR *dh = opendir(path_elegido);
	while((d = readdir(dh)) != NULL){
		if(string_contains(d->d_name, ".ims")){
			path_recurso = string_duplicate(path_elegido);
			string_append(&path_recurso, "/");
			string_append(&path_recurso, d->d_name);
			recurso = config_create(path_recurso);
			if(config_get_int_value(recurso, "SIZE") > 0)
				chequear_blocks_data(recurso, LOGGEAR_CAMBIOS);
			chequear_file_size(recurso);
			chequear_block_count(recurso, LOGGEAR_CAMBIOS);
			free(path_recurso);
			config_destroy(recurso);
		}
	}
	closedir(dh);
}


void recuperar_fs(int received_signal){
	pthread_mutex_lock(&mutex_blocks);
	log_info(logger, "\033[0;31mSe inicio el protocolo fsck\033[0m.");
	chequear_files(path_files);
	chequear_superbloque();
	log_info(logger, "\033[0;32mFin del protocolo.\033[0m");
	pthread_mutex_unlock(&mutex_blocks);
}


char* generar_md5(char* datos, size_t tamanio_datos){
	char* md5 = malloc(32 + 1);
	char* datos_aux = malloc(tamanio_datos);
	memcpy(datos_aux, datos, tamanio_datos);
	if(datos[tamanio_datos-1] != '\0'){	
		datos_aux= realloc(datos_aux, tamanio_datos + 1);
		datos_aux[tamanio_datos] = '\0';
	}
	char* md5sum_command = string_from_format("echo -n \"%s\" | md5sum\0", datos_aux);
	FILE* pipe = popen(md5sum_command, "r");
	if(fgets(md5, 32 + 1, pipe) == NULL)
		perror("Error al generar MD5");
	pclose(pipe);
	
	free(datos_aux);
	free(md5sum_command);
	
	return md5;

}