/*
 * file_system.c
 *
 *  Created on: 20 jun. 2021
 *      Author: utnso
 */

#include "file_system.h"
#include "superbloque.h"
#include "fs_paths.h"
#include "../../shared/include/shared_utils.h"

#include <commons/config.h>
#include <commons/string.h>
#include <commons/bitarray.h>
#include <commons/collections/list.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>
#include <unistd.h>
#include <inttypes.h>
#include <errno.h>
#include <signal.h>
#include <dirent.h>
#include <stdio.h>
#include <ctype.h>

#define FILE_FORMAT "SIZE=\nBLOCK_COUNT=\nBLOCKS=\nCARACTER_LLENADO=\nMD5_ARCHIVO=\n"

//variables globales
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

//declaracion de funciones locales usadas en inicializacion
void recuperar_fs(int);
void iniciar_en_limpio();

void generar_directorios(char*);
void generar_superbloque();
void generar_blocks();
char* generar_md5(char*, size_t);

//Funciones
void incializar_fs(){
	logger = log_create(PATH_LOGGER, "I-Mongo-Store", 1, LOG_LEVEL_INFO);
	config = config_create(PATH_CONFIG);
	pthread_mutex_init(&mutex_blocks, NULL);
	iniciar_en_limpio();
	
	signal(SIGUSR1, recuperar_fs);
	iniciar_servidor(logger);
	while(1){
		sleep(1000);
	};

}


//Funcion que genera las estructuras necesarias
void iniciar_en_limpio(){
	path = config_get_string_value(config, "PUNTO_MONTAJE");

	generar_directorios(path);
	generar_superbloque();
	generar_blocks();
	log_info(logger, "El sistema se ha inicializado correctamente!");	

}


void crear_directorio(char* path){
	int chequear_error;

	chequear_error = mkdir(path, 0777);

	//TODO : Manejar con logger
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

	uint32_t blocks = config_get_int_value(config, "BLOCKS");
	uint32_t block_size = config_get_int_value(config, "BLOCKS_SIZE");
	uint32_t blocks_en_bytes = blocks_to_bytes(blocks);

	char* puntero_a_bits = calloc(blocks_en_bytes, sizeof(char));

	size_t tamanio_sb = len(blocks);

	//Setea su tamaño para poder mapearlo
	if(ftruncate(sb_file, tamanio_sb) == -1){
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
	int blocks = config_get_int_value(config, "BLOCKS");
	int block_size = config_get_int_value(config, "BLOCKS_SIZE");

	while(1){
		pthread_mutex_lock(&mutex_blocks);
		msync(blocks_p, blocks * block_size, MS_SYNC);
		pthread_mutex_unlock(&mutex_blocks);
		sleep(sync_time);
	}
}

void generar_blocks(){
	int b_file;
	
	path_blocks = string_duplicate(path);
	string_append(&path_blocks, "/Blocks.ims");

	b_file = open(path_blocks, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

	int blocks = config_get_int_value(config, "BLOCKS");
	int block_size = config_get_int_value(config, "BLOCKS_SIZE");

	if(lseek(b_file, 0, SEEK_END) > 1){
		log_info(logger, "Se recupero un Blocks existente.");
	}else{
		log_info(logger, "Se genero el archivo Blocks.");
		if(ftruncate(b_file, blocks * block_size)==-1){
			perror("Error al generar la metadata de un archivo.");
		}
		fsync(b_file);
	}

	blocks_p = (char*) mmap(NULL, blocks * block_size, PROT_WRITE | PROT_READ, MAP_SHARED, b_file, 0);
	close(b_file);

	pthread_create(&hilo_blocks, NULL, (void*) actualizar_blocks, NULL);
	pthread_detach(hilo_blocks);

}

char* generar_template_meta_file (char* filename){
	char* file_content;
	int meta_file;

	char* path_file = string_duplicate(path_files);
	string_append(&path_file, "/");
	string_append(&path_file, filename);

	meta_file = open(path_file, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	if(ftruncate(meta_file, strlen(FILE_FORMAT))==-1){
		perror("Error al generar la metadata de un archivo.");
	}
	fsync(meta_file);
	file_content = mmap(NULL, strlen(FILE_FORMAT), PROT_WRITE | PROT_READ, MAP_SHARED, meta_file, 0);
	strcpy(file_content, FILE_FORMAT);
	munmap(file_content, strlen(FILE_FORMAT));
	close(meta_file);
	return path_file;
}

void generar_recurso(){
	//TODO
}

void generar_bitacora(){
	//TODO
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

void limpiar_lista_bloques(t_list* lista_bloques){
	t_link_element* elemento_actual;
	elemento_actual = lista_bloques->head;
	for (int i=0; i < list_size(lista_bloques);i++){
		free(elemento_actual->data);
		elemento_actual = elemento_actual->next;
	}
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
	uint32_t block_size;
	
	int offset = 0;
	memcpy(&cant_supuesta_bloques, superbloque, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&block_size, superbloque + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	
	cant_real_bloques = blocks_file_size / block_size;

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

void chequear_block_count(t_config* recurso){
	int len_blocks;
	char** blocks = config_get_array_value(recurso, "BLOCKS");
	char* int_as_array;
	for(len_blocks=0; blocks[len_blocks] != NULL; len_blocks++);
	if(config_get_int_value(recurso, "BLOCK_COUNT") != len_blocks){
		int_as_array = string_from_format("%d", len_blocks);
		config_set_value(recurso, "BLOCK_COUNT", int_as_array);
		config_save(recurso);
		free(int_as_array);
		log_info(logger, "Se corrigio un sabotaje en la cantidad de bloques del archivo: %s", recurso->path);
	}
	for(len_blocks=0; blocks[len_blocks] != NULL; len_blocks++){
		free(blocks[len_blocks]);
	}
	free(blocks);
}

void chequear_file_size(t_config* recurso){
	int block_size = config_get_int_value(config, "BLOCKS_SIZE");
	char** blocks = config_get_array_value(recurso, "BLOCKS");
	int size_supuesto = config_get_int_value(recurso, "SIZE");
	int offset;
	int current_block;
	int size = 0;
	char* size_as_array;
	pthread_mutex_lock(&mutex_blocks);
	for(int i = 0; blocks[i] != NULL; i++){
		current_block = atoi(blocks[i]);
		for(offset = 0; offset<block_size && blocks_p[current_block * block_size + offset] != 0; offset++){
			size++;
		}
		free(blocks[i]);
	}
	pthread_mutex_unlock(&mutex_blocks);
	if(size_supuesto != size){
		size_as_array = string_from_format("%d", size);
		config_set_value(recurso, "SIZE", size_as_array);
		config_save(recurso);
		free(size_as_array);
		log_info(logger, "Se corrigio un sabotaje en el tamanio del archivo: %s", recurso->path);
	}
	free(blocks);
}

char* get_blocks_data (t_config* recurso){
	int block_size = config_get_int_value(config, "BLOCKS_SIZE");
	int file_size = config_get_int_value(recurso, "SIZE");
	char** blocks = config_get_array_value(recurso, "BLOCKS");
	char* blocks_data = calloc(file_size, sizeof(char));
	int current_block;
	int offset;
	int bytes_chequeados = 0;
	pthread_mutex_lock(&mutex_blocks);
	for (int i= 0; blocks[i] != NULL; i++){
		current_block = atoi(blocks[i]);
		for (offset=0; offset < block_size && bytes_chequeados < file_size; offset++){
			memcpy((void*) blocks_data + bytes_chequeados, (void*) blocks_p + current_block * block_size + offset, sizeof(char));
			bytes_chequeados++;
		}
		free(blocks[i]);
	}
	pthread_mutex_unlock(&mutex_blocks);
	free(blocks);
	return blocks_data;
}

void restaurar_archivo(t_config* recurso){
	int block_size = config_get_int_value(config, "BLOCKS_SIZE");
	char** blocks = config_get_array_value(recurso, "BLOCKS");
	int file_size = config_get_int_value(recurso, "SIZE");
	char* aux = config_get_string_value(recurso, "CARACTER_LLENADO");
	char caracter_de_llenado = aux[0];
	int offset;
	int i;
	int bytes_escritos = 0;
	int current_block;
	pthread_mutex_lock(&mutex_blocks);
	//Primero limpio los bloques
	for (i = 0; blocks[i] != NULL; i++){
		offset = block_size * atoi(blocks[i]);
		memset(blocks_p + offset, 0, block_size);
	}
	//Luego los escribo hasta completar el size
	for(i = 0; blocks[i] != NULL; i++){
		current_block = atoi(blocks[i]);
		for(offset = 0; offset < block_size && bytes_escritos < file_size; offset++){
			blocks_p [current_block * block_size + offset] = caracter_de_llenado;
			bytes_escritos++;
		}
		free(blocks[i]);
	}
	pthread_mutex_unlock(&mutex_blocks);
	free(blocks);
}

void chequear_blocks_data(t_config* recurso){
	char* blocks_data = get_blocks_data(recurso);
	char* md5_data = generar_md5 (blocks_data, config_get_int_value(recurso, "SIZE"));
	char* md5_archivo = config_get_string_value(recurso, "MD5_ARCHIVO");
	if(strcmp(md5_data, md5_archivo) != 0){
		restaurar_archivo(recurso);
		char* new_blocks_data = get_blocks_data(recurso);
		char* new_md5 = generar_md5(new_blocks_data, config_get_int_value(recurso, "SIZE"));
		config_set_value(recurso, "MD5_ARCHIVO", new_md5);
		config_save(recurso);
		log_info(logger, "La data del archivo %s no coincidia con el contenido del archivo Blocks: se restauro el archivo.", recurso->path);
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
			chequear_file_size(recurso);
			chequear_block_count(recurso);
			if(config_get_int_value(recurso, "SIZE") > 0)
				chequear_blocks_data(recurso);
			free(path_recurso);
			config_destroy(recurso);
		}
	}
	closedir(dh);
}

void recuperar_fs(int received_signal){
	log_info(logger, "\033[0;31mSe inicio el protocolo fsck\033[0m.");
	chequear_files(path_files);
	chequear_superbloque();
	log_info(logger, "\033[0;32mFin del protocolo.\033[0m");
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

/*
void limpiar_bitmap(t_bitarray* bitmap){
	for(int i = 0; i < ((bitmap-> size) * 8); i++ ){
		bitarray_clean_bit(bitmap, i);
	}
}

void mostrar_bitmap(t_bitarray* bitmap){
	for(int i = 0; i < ((bitmap-> size) * 8); i++ ){
		printf("%d", bitarray_test_bit(bitmap, i));
	}
	printf("\n");
}
*/
