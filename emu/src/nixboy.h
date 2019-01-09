#ifndef NIXBOY_H
#define NIXBOY_H

#include "nb_gfx.h"
//types and functions are prefix with nb, nixboy

typedef struct nb_cartridge
{
    //All references are non-owning to allow swapping 
    byte * palette;  // 256 24 bit  Color values 
    byte * tiles;   //512 tiles tile sheet 8 bit 8x8 240x160 color grids  
    byte * map;     //tile map 8 bit 240x160 tile indexes
}nb_cartridge ;

// There exists a texture object for most data blocks in the cartidge for render purposes



#endif // NIXBOY_H
