#ifndef NB_TYPES_H
#define NB_TYPES_H

typedef unsigned char	byte ;

typedef struct color
{
    union
    {
        struct {byte  r,g,b;};
        byte      data[3];
    };        
}color;


#endif // NB_TYPES_H

