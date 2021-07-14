#ifndef INSTRUCCION_H_
#define INSTRUCCION_H_

#include <stdlib.h>
#include <commons/string.h>

typedef struct{
	char instruccion[20];
	char nombre_recurso[20];
	char caracter_de_llenado;
	void (*funcion) (char*, int);
}t_instruccion;


t_instruccion* get_instruccion (char* instruccion, t_instruccion tabla_comandos[]);

void ejecutar_instruccion (t_instruccion* instruccion, int cantidad);

#endif