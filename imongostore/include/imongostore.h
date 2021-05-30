#ifndef IMONGOSTORE_H
#define IMONGOSTORE_H
#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include "shared_utils.h"
#include "tests.h"
#include <commons/string.h>

typedef struct tareas{
    int pid; //numero de patota
    t_list* tareas_tripu;
}t_tareas;

t_tareas* TAREAS_GLOBAL;


#endif