#include "nb.h"
#define DEFAULT_TITLE                "nixboy"
//width and height of regular sprite, wide and tall double with and height respectively
#define DEFAULT_SPRITE_SIZE          8       
//Number of colors
#define DEFAULT_PALETTE_SIZE         32      

#define DEFAULT_SPRITE_ATLAS_WIDTH   64   
#define DEFAULT_SPRITE_ATLAS_HEIGHT  64   

#define DEFAULT_TILE_ATLAS_WIDTH     128  
#define DEFAULT_TILE_ATLAS_HEIGHT    128  

#define DEFAULT_SCREEN_WIDTH         100  
#define DEFAULT_SCREEN_HEIGHT        60 

#define DEFAULT_COLOR_DEPTH          255
#define DEFAULT_SCALE                10 

#define DEFAULT_MAX_SPRITE_COUNT     256




int main(int argc, char ** argv)
{
    //should load  data from from cart
    rgb  default_palette_colors[DEFAULT_PALETTE_SIZE] = {
#include "grayscale_palette.inl"
//#include "default_palette.inl"
    };



    byte default_sprite_atlas_indices[ DEFAULT_SPRITE_ATLAS_WIDTH * DEFAULT_SPRITE_ATLAS_HEIGHT] = {
#include "checkered_sprite_atlas.inl"
//#include "default_sprite_atlas.inl"
    };
    //
    nb_settings settings = 
    {
        cartridge_filepath : "res/test.nbc" ,

        .screen = 
        {
            title : DEFAULT_TITLE        ,
            width : DEFAULT_SCREEN_WIDTH ,
            height:DEFAULT_SCREEN_HEIGHT ,
            scale :DEFAULT_SCALE
        },    

        .gfx =
        {
            palette_size          : DEFAULT_PALETTE_SIZE         ,
            sprite_size           : DEFAULT_SPRITE_SIZE         , 
            sprite_atlas_width    : DEFAULT_SPRITE_ATLAS_WIDTH  ,
            sprite_atlas_height   : DEFAULT_SPRITE_ATLAS_HEIGHT ,
            tile_atlas_width      : DEFAULT_TILE_ATLAS_WIDTH    ,
            tile_atlas_height     : DEFAULT_TILE_ATLAS_HEIGHT  ,
            color_depth           : DEFAULT_COLOR_DEPTH          ,
            max_sprite_count      : DEFAULT_MAX_SPRITE_COUNT
        }
    } ;


    //parse command line options, modify settings
    nb_debug("Starting nixboy\n");
    nb_startup(&settings);


    nb_debug("Initializing memory\n");
    nb_set_palette(&default_palette_colors[0]);    
    nb_set_sprite_atlas(&default_sprite_atlas_indices[0]);
    //nb_set_tile_atlas(&default_tile_atlas_indices[0]);

    
    nb_debug("Creating sprites\n");
    nb_sprite * sprite0;
    nb_sprite * sprite1;
    sprite0 = nb_add_sprite(NB_SPRITE_REGULAR, 0);
    nb_set_sprite_xy(sprite0,0,0);

    
    sprite1 = nb_add_sprite(NB_SPRITE_REGULAR, 1);
    nb_set_sprite_xy(sprite1,DEFAULT_SPRITE_SIZE,0);
        

    //returns if paused, continue.
    nb_status status;


    float offsetx, offsety;
    u32 fps_cap = 200;
    nb_cap_fps(fps_cap);


    nb_debug("Starting machine \n");

    while( NB_CONTINUE ==  (status = nb_update() ) )
    {   
        u32 flags =0 ; 
        set_flag(flags , NB_FLAG_SPRITE_ATLAS_DIRTY);
        set_flag(flags , NB_FLAG_PALETTE_DIRTY);
     
        //on key press. TODO Create an event callback?   
        if(nb_get_key_state(NB_q) == NB_KEYPRESS)
        {
            sprite1->offset.x +=1;            
        }        
        nb_draw(  flags  );
    } 

    nb_shutdown();

    return 0;
}
