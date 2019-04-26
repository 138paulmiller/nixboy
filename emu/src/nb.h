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
Rendering 
	Palette      : [ c0 | c1 | .... cT] 

		where ci for i from 0 to T is an RGBA color
	
	Sprite Atlas : 

*/

//Len is number of units. Size is number of bytes
//sprites are referenced
#ifndef NIXBOY_H
#define NIXBOY_H

#include "nb_fs.h"
#include "nb_gc.h"
#include "nb_gfx.h"
#include "nb_cart.h"




#define nb_test(flags,pos) (( (flags) & (1<<pos ) )>>pos)
#define nb_flag(flags,pos) ( (flags) = (flags) | (1<<pos));


typedef enum nb_flags
{
	//indicates bit position in flag
	NB_FLAG_SPRITE_PALETTE_DIRTY 		= 0,
	NB_FLAG_TILE_PALETTE_DIRTY 		= 1,
	NB_FLAG_SPRITE_ATLAS_DIRTY 	= 2,
	NB_FLAG_TILE_ATLAS_DIRTY 	= 3,
	NB_FLAG_LEVEL_DIRTY 	= 4
}
nb_flags;

typedef enum nb_shader_index
{
	//indicates bit position in flag
	NB_SPRITE_SHADER = 0,
	NB_TILE_SHADER,
	NB_SHADER_COUNT  
}
nb_shader_index;


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
		u16 		sprite_width;  
		u16 		sprite_height; 
		u16 		sprite_atlas_width;
		u16 		sprite_atlas_height; 
		u16 		tile_atlas_width;
		u16 		tile_atlas_height;  
		u16 		level_width;
		u16 		level_height; 
		u16 		color_depth;
		u16 		max_sprite_count;
	}gfx;


} nb_settings;




//STate of nb
typedef struct nb_state
{

	//************** gfx state
	struct
	{

		nb_shader   shaders[NB_SHADER_COUNT];
	
		//Maintains references to corresponding palettes and atlas textures 
		nb_atlas sprite_atlas;
		nb_atlas tile_atlas;

		nb_palette sprite_palette;
		nb_palette tile_palette;
		
		nb_level level;   
	} gfx;
	
	//Data segments. Owningg pointers!
    
    struct
    {
    	rgb  * tile_palette_colors;
    	rgb  * sprite_palette_colors;
    	byte * sprite_atlas_indices;
    	byte * tile_atlas_indices;
    	byte * level_indices;		//atlas that indexes into tile atlas 
    	nb_sprite * sprite_table;
    } ram;

    struct
    {

		//cache vars
		u32 sprite_table_size; //max number of sprites 
		////////////////// 	uniforms /////////////////// 
		vec2i sprite_resolution; //number of pixels along sprite width|height 
		vec2i screen_resolution;
		vec2i sprite_atlas_resolution;
		vec2i tile_atlas_resolution;
		vec2i level_resolution;

		u32 palette_size ;		//dwisth of palette in colors
		u32 color_depth;
		u32 screen_scale;
		//////////////////////// ram sizes cache ////////////////
		u32 sprite_table_block_size;
	    u32 palette_block_size     ;
	    u32 sprite_atlas_block_size;
	    u32 tile_atlas_block_size ;
	    u32 level_block_size ;


    } cache;

}nb_state;


void 		nb_startup(nb_settings * settings);

/*
	Updates machine
		- 	update input
		-	update cpu instructions
		- 	update state
*/

nb_status 	nb_update();


/* - 	render state */
nb_status 	nb_draw(u32 flags);
/*
*/
void 		nb_shutdown();


//Copies data over into memory
void        nb_set_tile_palette(rgb * colors);
rgb *       nb_get_tile_palette();

void        nb_set_sprite_palette(rgb * colors);
rgb *       nb_get_sprite_palette();

//Copies data over into memory
void        nb_set_sprite_atlas(byte * color_indices);
byte *      nb_get_sprite_atlas();


//Copies data over into memory
void        nb_set_tile_atlas(byte * color_indices);
byte *      nb_get_tile_atlas();

void        nb_set_level(byte * level_indices);
byte *      nb_get_level();



//-------------------- Creation/ Deletion utilities -------------

//Create a mapping to map sprite index to a 2D offset
//sprites are tiles whose positions are not determined by an index, but an offset
nb_sprite *	nb_add_sprite  (    nb_sprite_type type,   int index );
void 		nb_remove_sprite(  nb_sprite * sprite);



//Create helpers to swapping palettes, updating sheets etc
//Loads the palette and a texture to render .
//void nb_init_palette(nb_palette * palette);

//Update the palettes texture .


#endif // NIXBOY_H
