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
#include "nb_cart.h"



//-------------------------- memory defs --------------------------




typedef struct nb_settings
{

	//path to cartidge file. loads memory and code data from the formatted text 
	str 	cartridge_filepath ;
	struct _screen
	{
		//screen settings
		str 	title;
		u16 		width;
		u16 		height;
		u16 		scale;
	} screen;

	struct _gfx
	{
		//gfx settings
		u16 		palette_size; 
		u16 		sprite_size; 
		u16 		sprite_atlas_width;
		u16 		sprite_atlas_height; 
		u16 		tile_atlas_width;
		u16 		tile_atlas_height; 
		u16 		color_depth;
		u16 		max_sprite_count;
	}gfx;


} nb_settings;




//STate of nb
typedef struct nb_state
{

	//************** gfx state

		nb_shader   sprite_shader;
			//TODO
		nb_shader   tile_shader;

		//Maintains references to corresponding palettes and atlas textures 
		nb_atlas sprite_atlas;
		nb_atlas tile_atlas;

		nb_palette palette;
	//Data segments. Owningg pointers!
    rgb  * palette_colors;
    byte * sprite_atlas_indices;
    nb_sprite * sprite_block;

    //cache vars
	//cache 
	vec2i screen_resolution;
	vec2i sprite_atlas_resolution;
	vec2i tile_atlas_resolution;

    u32 sprite_block_size; //size of sprite object buffer
    u32 sprite_size;
	u32 palette_size ;

}nb_state;


void 		nb_startup(nb_settings * settings);

/*
	Updates machine
		- 	update input
		-	update cpu instructions
		- 	update state
*/

nb_status 	nb_update();




#define test_flag(flags,pos) ( (flags) & (1<<pos ) )
#define set_flag(flags,pos) (flags |= (1>> pos));


typedef enum nb_flags
{
	//indicates bit position in flag
	NB_FLAG_PALETTE_DIRTY 		= 1,
	NB_FLAG_SPRITE_ATLAS_DIRTY 	= 2
}
nb_flags;



/* - 	render state */
nb_status 	nb_draw(u32 flags);
/*
*/
void 		nb_shutdown();


//Copies data over into memory
void        nb_set_palette(rgb * colors);
rgb *       nb_get_palette();

//Copies data over into memory
void        nb_set_sprite_atlas(byte * color_indices);
byte *      nb_get_sprite_atlas();


//Copies data over into memory
void        nb_set_tile_atlas(byte * color_indices);
byte *      nb_get_tile_atlas();



//-------------------- Creation/ Deletion utilities -------------

//Create a mapping to map sprite index to a 2D offset
//sprites are tiles whose positions are not determined by an index, but an offset
nb_sprite *	nb_add_sprite  (    nb_sprite_type type,   int index );
void 		nb_remove_sprite(  nb_sprite * sprite);



//Create helpers to swapping palettes, updating sheets etc
//Loads the palette and a texture to render .
//void nb_init_palette(nb_palette * palette);

//Update the palettes texture .
//void nb_update_palette(nb_palette * palette);


#endif // NIXBOY_H
