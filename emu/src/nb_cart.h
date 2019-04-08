#include "nb_def.h"

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

    byte * level_palette;  		//  
    byte * level_atlas;   	//   
    byte * level_map;     	// 
 
    byte * sprite_palette;       //  
    byte * sprite_atlas;   	//   
    byte * sprite_map;     	// 

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
