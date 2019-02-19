#include "nb_fs.h"
#include "nb_gc.h"
#include <stdio.h>


void nb_fread(const char * streamname, char ** contents, int * size)
{
	int stream_size = 0;
	char * bytes = 0;
	//open fle
	FILE * stream = fopen(streamname, "rb");
	if(! stream )
	{
		*contents = 0;
		size = 0 ;
		return;
	}
	//move to end of stream
	fseek(stream, 0L, SEEK_END);
	//get cursor stream_size
	stream_size = ftell(stream);
	//move to beg of stream
	fseek(stream, 0L, SEEK_SET);
	//Allocate enough memory (should I add 1 for the \0?)
	bytes = (char *)nb_malloc(stream_size);
	bytes[stream_size] = '\0';
	fread(bytes, stream_size, 1, stream);
	
	*contents = bytes;
	*size = stream_size;
}


