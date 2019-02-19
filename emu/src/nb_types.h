#ifndef NB_TYPES_H
#define NB_TYPES_H
/*
	Primitive type defintions 
*/
#include <stdio.h>
#include <stdint.h>


typedef uint8_t		byte ;
typedef uint8_t		bool ;
typedef uint16_t	u16;
typedef uint32_t	u32;
typedef int16_t		i16;
typedef int32_t		i32;

typedef struct rgb
{
    union
    {
        struct {byte  r,g,b;};
        byte      data[3];
    };        
}rgb;

typedef struct vec2b
{
	byte x,y;      
}vec2b;

typedef struct vec2i
{
	i32  x,y;      
}vec2i;

typedef struct vec2f
{
	float x,y;      
}vec2f;

typedef struct vec3i
{
    i32  x,y,z;        
}vec3i;

//TODO Flush 
//Create a logging system with varying severities. Fatal crashes
#define nb_error(...) 	{printf(__VA_ARGS__);exit(-1);	}
#define nb_warn(...) 	{printf("Warn:"__VA_ARGS__);				}
#define nb_log(...) 	{printf(__VA_ARGS__);				}


#endif // NB_TYPES_H

