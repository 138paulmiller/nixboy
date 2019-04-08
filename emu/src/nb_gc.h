#ifndef NB_GC_H
#define NB_GC_H

#include <stdlib.h>
#include "nb_def.h"
/*
	Garbage collection
*/
void * 	nb_malloc(u32 size) ;

void 	nb_free(void * data) ;

#endif 