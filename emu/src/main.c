#include "nb.h"

////////////// ----------------- Default Configs ---------------------------

#define DEFAULT_TITLE                "nixboy"
//width and height of regular sprite, wide and tall double with and height respectively
#define DEFAULT_SPRITE_WIDTH          8       
#define DEFAULT_SPRITE_HEIGHT         8       
//Number of colors
#define DEFAULT_PALETTE_SIZE         32     

#define DEFAULT_SPRITE_ATLAS_WIDTH   64   
#define DEFAULT_SPRITE_ATLAS_HEIGHT  64   

#define DEFAULT_TILE_ATLAS_WIDTH     64  
#define DEFAULT_TILE_ATLAS_HEIGHT    64

//number of tile width and tile height for a level
#define DEFAULT_TILEMAP_WIDTH     64  
#define DEFAULT_TILEMAP_HEIGHT    64
  
#define DEFAULT_TILE_HEIGHT          8  
#define DEFAULT_TILE_WIDTH           8  

#define DEFAULT_SCREEN_WIDTH         124  
#define DEFAULT_SCREEN_HEIGHT        64

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

    byte default_tile_atlas_indices[ DEFAULT_SPRITE_ATLAS_WIDTH * DEFAULT_SPRITE_ATLAS_HEIGHT] = {
#include "gradient_tile_atlas.inl"
    };
    //



    byte default_tilemap_indices[ DEFAULT_TILEMAP_WIDTH * DEFAULT_TILEMAP_HEIGHT] = {
#include "checkered_tilemap.inl"
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
            palette_size          : DEFAULT_PALETTE_SIZE        ,
            sprite_width          : DEFAULT_SPRITE_WIDTH         , 
            sprite_height         : DEFAULT_SPRITE_HEIGHT         , 
            sprite_atlas_width    : DEFAULT_SPRITE_ATLAS_WIDTH  ,
            sprite_atlas_height   : DEFAULT_SPRITE_ATLAS_HEIGHT ,
            tile_atlas_width      : DEFAULT_TILE_ATLAS_WIDTH    ,
            tile_atlas_height     : DEFAULT_TILE_ATLAS_HEIGHT   ,
            tile_width           : DEFAULT_TILE_WIDTH         , 
            tile_height          : DEFAULT_TILE_HEIGHT        , 
            tilemap_width           : DEFAULT_TILEMAP_WIDTH         , 
            tilemap_height          : DEFAULT_TILEMAP_HEIGHT        , 
            color_depth           : DEFAULT_COLOR_DEPTH         ,
            max_sprite_count      : DEFAULT_MAX_SPRITE_COUNT
        }
    } ;

    //parse command line options, modify settings
    nb_debug("Starting nixboy\n");
    nb_startup(&settings);


    nb_debug("Initializing memory\n");
    nb_set_sprite_palette ( &default_palette_colors[0]        );
    nb_set_sprite_atlas   ( &default_sprite_atlas_indices[0]  );
    
    nb_set_tile_palette   ( &default_palette_colors[0]        );    
    nb_set_tile_atlas     ( &default_tile_atlas_indices[0]    );
        
    nb_set_tilemap          ( &default_tilemap_indices[0]         );
    
    nb_debug("Creating sprites\n");
    nb_sprite * sprite0;
    nb_sprite * sprite1;
    sprite0 = nb_add_sprite(NB_SPRITE_REGULAR, 0);
    nb_set_sprite_xy(sprite0,0,0);
    nb_set_sprite_offset(sprite0, 0, 0);
    
    sprite1 = nb_add_sprite(NB_SPRITE_REGULAR, 1);
    nb_set_sprite_xy(sprite1,     settings.gfx.sprite_width, 0 );
    nb_set_sprite_offset(sprite1, settings.gfx.sprite_width, 0);
        
    //returns if paused, continue.
    nb_status status;


    float offsetx=0, offsety=0;
    u32 fps_cap = 200;
    nb_cap_fps(fps_cap);

    rgb color = {0,0,0};
    u32 color_i = 0;

    nb_debug("Starting machine \n");

    u32 draw_flags; 
    while( NB_CONTINUE ==  (status = nb_update() ) )
    {   
        float dx=0, dy=0;
        draw_flags =0 ;
        //on key press. TODO Create an event callback?   
        if(nb_get_keystate(NB_ESCAPE) == NB_KEYPRESS)
        {
            break;
        }  
        if(nb_get_keystate(NB_w) == NB_KEYPRESS)
        {
            dy=1;
        }   
        if(nb_get_keystate(NB_s) == NB_KEYPRESS)
        {
            dy=-1;
        }
        if(nb_get_keystate(NB_d) == NB_KEYPRESS)
        {
            dx=1;
        }        
        if(nb_get_keystate(NB_a) == NB_KEYPRESS)
        {
            dx=-1;
        }

        if(nb_get_keystate(NB_i) == NB_KEYPRESS)
        {
            offsety +=1;
            nb_scroll(0,1);
        }   
        if(nb_get_keystate(NB_k) == NB_KEYPRESS)
        {
            offsety -=1;     
            nb_scroll(0,-1);       
        }
        if(nb_get_keystate(NB_j) == NB_KEYDOWN)
        {
            offsetx -=1;         
            nb_scroll(-1,0);   
        }        
        if(nb_get_keystate(NB_l) == NB_KEYDOWN)
        {
            offsetx +=1;  
            nb_scroll(1,0);          
        }

        if(nb_get_keystate(NB_SPACE) == NB_KEYPRESS)
        {
            color_i ++;
            nb_debug("color index : %d\n", color_i);
        }
        if(nb_get_keystate(NB_r) == NB_KEYDOWN)
        {
            color.r ++;
            color.r = color.r % settings.gfx.color_depth;
                        
            nb_get_sprite_palette()[color_i].r++;
            nb_flag(draw_flags , NB_FLAG_SPRITE_PALETTE_DIRTY);
        }

        offsetx = clamp(offsetx, 0,  settings.gfx.palette_size  - settings.gfx.sprite_width);
        offsety = clamp(offsety, 0,  settings.gfx.palette_size - settings.gfx.sprite_height);           
        
        nb_set_sprite_offset(sprite1, offsetx, offsety);

        nb_move_sprite(sprite1, dx, dy);

        
        nb_draw(  draw_flags  );
    } 

    nb_shutdown();

    return 0;
}
