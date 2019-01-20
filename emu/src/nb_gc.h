#ifndef NB_GC_H
#define NB_GC_H

#include <stddef.h>
#include "nb_types.h"
/*
	Garbage collection
*/
void * 	nb_malloc(u32 size) ;

void 	nb_free(void * data) ;

#endif 