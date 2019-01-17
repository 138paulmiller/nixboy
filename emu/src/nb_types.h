#ifndef NB_TYPES_H
#define NB_TYPES_H
/*
	Primitive type defintions 
*/
#include <stdint.h>
typedef uint8_t			byte ;
typedef uint16_t		u16;
typedef uint32_t		u32;

typedef struct color
{
    union
    {
        struct {byte  r,g,b;};
        byte      data[3];
    };        
}color;


#endif // NB_TYPES_H

