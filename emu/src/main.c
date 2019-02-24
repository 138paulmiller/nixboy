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
    int scale =NB_SCALE;
    

    //parse cmd line args
    gfx_init(NB_TITLE, width*scale , height*scale );
    

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
#include "default_palette.inl"
    };
    gfx_palette palette0;
    gfx_init_palette(&palette0, &palette_data[0], palette_width, palette_height);


    int atlas_height = NB_SPRITE_ATLAS_SIZE;
    int atlas_width = NB_SPRITE_ATLAS_SIZE;

    byte atlas_indices[NB_SPRITE_ATLAS_SIZE*NB_SPRITE_ATLAS_SIZE] = {
#include "default_sprite_atlas.inl"
    };
    

    gfx_atlas sptite_atlas0;
    gfx_init_atlas(&sptite_atlas0, &atlas_indices[0], atlas_width, atlas_height);


    // Setup sheet
    gfx_sheet sheet0;
    gfx_init_sheet(&sheet0, &palette0, &sptite_atlas0);
    

 	//---------------------- init sprites --------------------------------

    
    vec2i offset = {0,0};
    gfx_sprite sprite0;
    gfx_init_sprite(&sprite0, &sheet0, &sprite_shader, offset, GFX_SPRITE_REGULAR);

    //set the current palette to the cpu palette
    cpu.palette = (byte*)palette_data;

    float offsetx, offsety;
    gfx_cap_fps(fps_cap);
    while(gfx_update())
    {

        //sprite0.offset.x ++;// (sprite0.offset.x++);
        sprite0.offset.y ++;// (sprite0.offset.y++);

            //sprite0.offset.x= sprite0.offset.x%(int)(sqrt(NB_ATLAS_SIZE));
        
//        gfx_set_sprite_xy(&sprite0, x, y);
 		//---------------------- draw sprites --------------------------------
        //only rebinds of not already bound


        gfx_bind_shader(&sprite_shader);
       
        gfx_use_sheet(&sheet0);

        //set textures units indices        
        gfx_set_uniform_int(&sprite_shader, GFX_UNIFORM_ATLAS,      GFX_ATLAS_TEXTURE_UNIT);
        gfx_set_uniform_int(&sprite_shader, GFX_UNIFORM_PALETTE,    GFX_PALETTE_TEXTURE_UNIT);

        gfx_set_uniform_vec2i(&sprite_shader, GFX_UNIFORM_RESOLUTION, &resolution);
        gfx_set_uniform_int(&sprite_shader, GFX_UNIFORM_SCALE, scale);

        gfx_set_uniform_int(&sprite_shader, GFX_UNIFORM_SPRITE_ATLAS_SIZE , NB_SPRITE_ATLAS_SIZE );

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
