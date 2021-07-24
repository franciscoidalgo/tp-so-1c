/*
 * superbloque.c
 *
 *  Created on: 21 jun. 2021
 *      Author: utnso
 */


#include "superbloque.h"
#include <stdio.h>
int blocks_to_bytes(int blocks){
	int aux = blocks / 8;
	return blocks % 8 == 0 ? aux : aux + 1;
}

size_t len(int blocks){
	size_t size = sizeof(uint32_t) * 2 + blocks_to_bytes(blocks);
	return size;
}
