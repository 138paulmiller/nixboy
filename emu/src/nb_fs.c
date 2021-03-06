#include "nb_fs.h"
#include "nb_gc.h"
#include <stdio.h>


void nb_fread(const str filepath, byte ** bytes, int * size)
{
	int stream_size = 0;
	str  buffer = 0;
	//open fle
	FILE * stream = fopen(filepath, "rb");


	if(! stream )
	{
		*bytes = 0;
		*size = 0 ;
		return;
	}
	//move to end of stream
	fseek(stream, 0L, SEEK_END);
	//get cursor stream_size
	stream_size = ftell(stream);
	//move to beg of stream
	rewind(stream);
	//Allocate enough memory (should I add 1 for the \0?)
	buffer = (str)nb_malloc(stream_size * sizeof(byte)+1);
	buffer[stream_size] = '\0';
	fread(buffer, sizeof(byte),stream_size, stream);
	fclose(stream);
	
	*bytes = buffer;
	*size = stream_size;
}


