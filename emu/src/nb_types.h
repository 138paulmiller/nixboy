#ifndef NB_TYPES_H
#define NB_TYPES_H
/*
	Primitive type defintions 
*/
#include <stdint.h>


typedef uint8_t		byte ;
typedef uint8_t		bool ;
typedef uint16_t	u16;
typedef uint32_t	u32;
typedef int16_t		i16;
typedef int32_t		i32;

typedef struct color
{
    union
    {
        struct {byte  r,g,b,a;};
        byte      data[4];
    };        
}color;

typedef struct vec2i
{
	i32  x,y;      
}vec2i;

typedef struct vec3i
{
    i32  x,y,z;        
}vec3i;


//Create a logging system with varying severities. Fatal crashes
#define nb_error(...) {printf(__VA_ARGS__);exit(-1);}


#endif // NB_TYPES_H

