#include "nb_defs.h"

#define NB_CODE_SEGMENT_SIZE    32768 /*32k bytes*/
/*

xMemory Bank

|---- | ---------------    |-------|
|     | Palette            | 768   |
|---- | ---------------    |-------|
|     | Level Atlas(Tiles) | 512   | 
|---- | ---------------    |-------|
|     | Level Map (Tiles)  | ????  | 
|---- | ---------------    |-------|
|     | Sprite Atlas       | ????  | 
|---- | ---------------    |-------|
|     | Sprite Map         | ????  |
*/
typedef struct nb_bank
{
    byte * palette;       //  Color palette
 
    byte * level_atlas;     //   "Super Texture" that compactly contains all tiles
    byte * level_map;       //  Level is maps from and index value to a tile in the atlas
 
    byte * sprite_atlas;   	//"Super" Texture that compactly contains all sprites.    

} nb_bank;

// 0 is default memory bank

typedef enum nb_bank_index
{
	NB_BANK_0 = 0 ,	
	NB_BANK_1     ,	
	NB_BANK_2     ,	
	NB_BANK_3 	  ,
 	NB_BANK_COUNT 
} nb_bank_index;


//Loads into memory. Cpu address memory banks when needed
typedef struct nb_cartridge
{
	byte code[NB_CODE_SEGMENT_SIZE];
 	nb_bank banks[NB_BANK_COUNT];
} nb_cartridge;




//***************************** Create Byte Code instruction set ***************************//
