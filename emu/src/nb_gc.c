#include "nb_gc.h"
/*
	Garbage collection
*/
void * nb_malloc(u32 size) 
{
	return malloc(size);
}
void 	nb_free(void * data) 
{
	free(data);
}
