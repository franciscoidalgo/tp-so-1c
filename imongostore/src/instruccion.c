#include "instruccion.h"

t_instruccion* get_instruccion (char* instruccion, t_instruccion tabla_comandos[]){
	int i;
	for(i =0; !string_equals_ignore_case(tabla_comandos[i].instruccion, instruccion) && tabla_comandos[i].funcion != NULL; i++);
	if(tabla_comandos[i].funcion == NULL)
		return (t_instruccion*) NULL;
	return &tabla_comandos[i];
}

void ejecutar_instruccion (t_instruccion* instruccion, int cantidad){
	(*(instruccion->funcion)) (instruccion->nombre_recurso, cantidad);
}
