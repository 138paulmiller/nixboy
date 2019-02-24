//types and functions are prefix with nb, nixboy

/*

___________RAM__________________
|Addr | Purpose        | Bytes |

x -----------------------------------Begin Bankable Memory
|---- | ---------------|-------|
|     | Palette        | 768   |
|---- | ---------------|-------|
|     | Tile Atlas     | 512   | 
|---- | ---------------|-------|
|     | Tile Map       | ????  | 
|---- | ---------------|-------|
|     | Sprite Atlas   | ????  | 
|---- | ---------------|-------|
|     | Sprite Map     | ????  |
x -----------------------------------End Bankable Memory
|---- | ---------------|-------|  
|     | Scroll Offset  | 4     |  
|---- | ---------------|-------|
|     | Screen         | 4800  |
|---- | ---------------|-------|

- Display is 240 x 160 4 bit colors
	
- Palette
	* 16, 32-bit color values
	* Bits per component R8G8B8A8
- Tile Atlas 
	* 256 x 256  4 bit color indices. Each byte indexes the color palette
	* Represents a texture atlas, composed of 8x8 tiles, allowing for 64 tiles in memory.
- Tile Map
	*  8 bit 256 x 256 tile indexes
	*	Defines Level
		- Level pixel size = 256*8 = 2048 x 2048
-Sprite Atlas
	* 4 bit 256x256 tile indexes
	* Sprite Modes 
	* Each tile/sprite is a rectangular collection of indices, either
		1. Regular 16x16
		2. Tall 16x32
		3. Wide 32x16
- Sprite Map
	* Defines any possible sprites rendered. Foreground tiles of varying size
		- Drawn in order, lowest address to highest
		- Each sprite is given a unique index into this array
		- Each Sprite Description block containts and screen x,y position, sprite type, and x,y position of bottom leftmost pixel in the sprite sheet,
	- Sprite Description block
	Addr:0   8   16  32  38 
	     ---------------------
	Id : | X | Y | U | V | T |  ....
	     ---------------------
		All Coordinates origin are bottom left
		X Y : position in level
		U,V : position on sprite sheet u,v in (256,256)
			Sprite Sheet is 
		T  : Sprite Type : Defined sizes above
		F  : 1 If Flipped, 0 if not

-------
2 Shaders
	Tile Sheet Shader
	Sprite Shader

*/

//Len is number of units. Size is number of bytes
//sprites are referenced
#ifndef NIXBOY_H
#define NIXBOY_H

#include "nb_fs.h"
#include "nb_gc.h"
#include "nb_gfx.h"
#include <math.h>


#define NB_PALETTE_SIZE  		32		//Number of colors
#define NB_SPRITE_ATLAS_SIZE  	64	// 64x64 sheet of indices
#define NB_MAP_ATLAS_SIZE  		128	//Number of colors
#define NB_SPRITE_SIZE  		8		//width and height of regular sprite, wide and tall double with and height respectively
#define NB_COLOR_DEPTH			255
#define NB_SCREEN_WIDTH  		100  
#define NB_SCREEN_HEIGHT 		60 

#define NB_SCALE 				10 
#define NB_TITLE 				"nixboy"



/*
Create a nb_rect that encapuslates verts,uvs.  texture  class that will render sprites, 
*/
typedef struct nb_cpu
{
    //All references are non-owning to allow onwing references to manage memory 
    byte * screen;  		// 
    byte * palette;  		//  
    byte * tile_atlas;   	//   
    byte * map;     		// 
}	nb_cpu ;

//Create Catridges that contain Memory Banks

// There exists a texture object for most data blocks in the cartidge for render purposes
typedef enum nb_tile_mode
{
	INVALID=-1, DEFAULT, SMALL, TALL, WIDE    
}	nb_tile_mode;


//Draw mode - Per pixel. Raw Set pixel functionality
void nb_draw_screen(nb_cpu * cpu);

//Loads the palette and a texture to render .
//void nb_init_palette(nb_palette * palette);

//Update the palettes texture .
//void nb_update_palette(nb_palette * palette);


#endif // NIXBOY_H
