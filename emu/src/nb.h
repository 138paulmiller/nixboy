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
|     | Tile Atlas     | ????  |
|---- | ---------------|-------|
|     | Sprite Sheet   | ????  | 
|---- | ---------------|-------|
|     | Sprite Map     | ????  |
x -----------------------------------End Bankable Memory

|---- | ---------------|-------|  
|     | Scroll Offset  | 4     |  
|---- | ---------------|-------|
|     | Screen         | 4800  |
|---- | ---------------|-------|

///Banks for Palette/Tile Atlas/Tile Map
- Display is 240 x 160 4 bit colors
	
- Palette
	* 16, 24-bit color values
	* Bits per component R8G8B8
- Tile Atlas 
	* 256 x 256  8 bit color indices. Each byte indexes the color palette
	* Represents a texture atlas, composed of 8x8 tiles, allowing for 64 tiles in memory.
- Tile Map
	*  8 bit 256 x 256 tile indexes
	*	Defines Level
		- Level pixel size = 256*8 = 2048 x 2048
-Sprite sheet
	* 8 bit 256x256 tile indexes
	* Sprite Modes 
	* Each tile/sprite is a rectangular collection of indices, either
		1. Default 8x8
		2. Small 4x4
		3. Tall 8x16
		4. Wide 18x8
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
*/

//Len is number of units. Size is number of bytes
//sprites are referenced
#ifndef NIXBOY_H
#define NIXBOY_H

#include "nb_gfx.h"
#include <math.h>

//Change to constants? Create static struct of constants  
#define NB_COLOR_DEPTH		  	24		//Size of each component
#define NB_PALETTE_SIZE  		16		//Number of colors
#define NB_SCREEN_WIDTH  		240  
#define NB_SCREEN_HEIGHT 		160 
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

//Used for rendering
typedef struct nb_palette
{
	color * data;  // 256 24 bit  Color values     
	gfx_texture texture;	
}	nb_palette;



//Draw mode - Per pixel. Raw Set pixel functionality
void nb_draw_screen(nb_cpu * cpu);

//Loads the palette and a texture to render .
//void nb_init_palette(nb_palette * palette);

//Update the palettes texture .
//void nb_update_palette(nb_palette * palette);


#endif // NIXBOY_H