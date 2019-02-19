/*
TODO	
	- Create (Tracing) Mark-Sweep Garbage collection system.
	Pointer struct
	See http://libcello.org/learn/garbage-collection
	- Create QuadTree for sprites. Use for rendering
	- Create "Batched" Rendering for multi-sheet levels?
	- Render targets. Deferred/Forward Rendering!
*/
/*
 


*/
#include "nb.h"

//



void nb_draw_screen(nb_cpu * cpu){}




int main(int argc, char ** argv)
{
    nb_cpu cpu;
    vec2i resolution = { NB_SCREEN_WIDTH, NB_SCREEN_HEIGHT};
    int width = NB_SCREEN_WIDTH, height = NB_SCREEN_HEIGHT;
    int fps_cap = 200;
    gfx_init(NB_TITLE, width*2, height*2);
    

    //  ---------------------- Compile Sprite Shader ----------------------
    gfx_shader sprite_shader;
    //create file struct, intermittengly check last modified data, if changes then reload (hotload ) 
    char *sprite_vert_source;
    int sprite_vert_size;
    char *sprite_frag_source;
    int sprite_frag_size;


    nb_fread("res/sprite.vert", &sprite_vert_source, &sprite_vert_size); 
    nb_fread("res/sprite.frag", &sprite_frag_source, &sprite_frag_size);
    
    if(! sprite_vert_source)
        nb_error("Failed to Read res/sprite.vert file")
    
    if(! sprite_frag_source)
        nb_error("Failed to Read res/sprite.frag file")
    

    gfx_init_shader(&sprite_shader, sprite_vert_source, sprite_frag_source);

    nb_free(sprite_vert_source);
    nb_free(sprite_frag_source);

 
    // ---------------------- Setup Palettes -----------------------------

    int i,j,x;


    //for rendering
    int palette_width = NB_PALETTE_SIZE;
    int palette_height = 1;
    rgb  palette_data[NB_PALETTE_SIZE] = {
        {0,0,0},{16,0,0},{32,0,0},{64,0,0},
        {80,0,0},{96,0,0},{112,0,0},{128,0,0},
        {144,0,0},{160,0,0},{176,0,0},{192,0,0},
        {208,0,0},{224,0,0},{240,0,0},{255,0,0}
    };
    gfx_palette palette0;
    gfx_init_palette(&palette0, &palette_data[0], palette_width, palette_height);


    int atlas_height = sqrt(NB_ATLAS_SIZE);
    int atlas_width = atlas_height;

    byte atlas_indices[NB_ATLAS_SIZE];


    //each palette color is a R8G8B8A8
    for(j=0; j < atlas_height; j++)
        for(i=0; i < atlas_width; i++)
        {
            x = ( (j*atlas_width)+i);
            atlas_indices[x] = x % NB_PALETTE_SIZE;
        }
    
    

    gfx_atlas atlas0;
    gfx_init_atlas(&atlas0, &atlas_indices[0], atlas_width, atlas_height);


    // Setup sheet
    gfx_sheet sheet0;
    gfx_init_sheet(&sheet0, &palette0, &atlas0);
    

 	//---------------------- init sprites --------------------------------

    
    vec2i offset;
    gfx_sprite sprite0;
    gfx_init_sprite(&sprite0, &sheet0, &sprite_shader, offset, GFX_SPRITE_REGULAR);

    //set the current palette to the cpu palette
    cpu.palette = (byte*)palette_data;


    gfx_cap_fps(fps_cap);
    while(gfx_update())
    {

        //physics step (delta)
        float x,y;
        gfx_get_sprite_xy(&sprite0, &x, &y);
        x+= gfx_delta_sec()*16;
sprite0.offset.x++;
sprite0.offset.x= sprite0.offset.x%(int)(sqrt(NB_ATLAS_SIZE));

//        gfx_set_sprite_xy(&sprite0, x, y);
 		//---------------------- draw sprites --------------------------------
        //only rebinds of not already bound


        gfx_bind_shader(&sprite_shader);
       
        gfx_use_sheet(&sheet0);
        
        gfx_set_uniform_int(&sprite_shader, GFX_UNIFORM_ATLAS,      GFX_ATLAS_TEXTURE_UNIT);
        gfx_set_uniform_int(&sprite_shader, GFX_UNIFORM_PALETTE,    GFX_PALETTE_TEXTURE_UNIT);
        gfx_set_uniform_vec2i(&sprite_shader, GFX_UNIFORM_RESOLUTION, &resolution);

        gfx_set_uniform_int(&sprite_shader, GFX_UNIFORM_PALETTE_SIZE , NB_PALETTE_SIZE );
        gfx_set_uniform_int(&sprite_shader, GFX_UNIFORM_COLOR_DEPTH  , NB_COLOR_DEPTH);

        gfx_render_sprite(&sprite0);
  //      nb_log("FPS\t%.03f\n", gfx_fps());
    } 
    gfx_destroy_palette(&palette0);
    gfx_destroy_sprite(&sprite0); 
    gfx_destroy(); 
    return 0;
}
