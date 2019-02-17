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
static const char * uniform_size        = "size";
static const char * uniform_pos         = "pos";
static const char * uniform_resolution  = "resolution";

//Maps -1.1 to width height
static const char * vertex_source = 
    "#version 130\n"
    "uniform ivec2 resolution;"
    "uniform vec2 size;"
    "uniform vec2 pos;"
    "in vec2 in_vert;"
    "in vec2 in_uv;"
    "out vec2 uv;"
    "void main(){"
        "uv=in_uv;\n"
        "vec2 new_pos = (size*in_vert+pos)/resolution*2.0-1.0;\n"
        "gl_Position =vec4(new_pos,0,1);\n"
    "}";

const char * fragment_source = 
    "#version 130\n"
    "uniform sampler2D sampler;"
    "in vec2 uv;"
    "out vec4 color;"
    "void main(){\n"
        "color = texture(sampler,uv);\n"
    "}";

void nb_draw_screen(nb_cpu * cpu){}


int main(int argc, char ** argv)
{
    nb_cpu cpu;
    vec2i resolution = { NB_SCREEN_WIDTH, NB_SCREEN_HEIGHT};
    int width = NB_SCREEN_WIDTH, height = NB_SCREEN_HEIGHT;
    int fps_cap = 200;
    gfx_init(NB_TITLE, width*2, height*2);
    
    gfx_shader sprite_shader;
    gfx_init_shader(&sprite_shader, vertex_source, fragment_source);
 
    // ---------------------- Setup Palettes -----------------------------

    color  * palette_data  = (color*)malloc(NB_PALETTE_SIZE * sizeof(color));
    color * c;
    int i,j,x;
    //for rendering
    int palette_height = 16;
    int palette_width = NB_PALETTE_SIZE/palette_height;
    int n=0;
    
    //each palette color is a R8G8B8A8
    for(j=0; j < palette_height; j++)
        for(i=0; i < palette_width; i++)
        {
            x = ( (j*palette_width)+i);
            c = &palette_data[x];
            c->r = x*6%255;
            c->g = 0;
            c->b = 0;
            c->a = 255;
        }
    
    gfx_texture palette_texture;
    
    if(gfx_init_texture(   &palette_texture, GFX_TEXTURE_2D, GFX_RGBA8,&palette_data->data[0],palette_width, palette_height)== GFX_FAILURE)
    {
        nb_error("Failed to init texture ");
    } 
    // Setup sheet
    gfx_sheet sheet0;
    sheet0.palette=  &palette_texture;
    

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

        gfx_set_sprite_xy(&sprite0, x, y);
 		//---------------------- draw sprites --------------------------------
        //only rebinds of not already bound
        gfx_bind_shader(&sprite_shader);
        gfx_set_uniform_vec2i(&sprite_shader, uniform_resolution, &resolution);
        
        //if dirty
        gfx_update_texture(  &palette_texture,0,0, palette_width, palette_height); 
        gfx_bind_texture(&palette_texture);

        gfx_render_sprite(&sprite0);
        nb_log("FPS\t%.03f\n", gfx_fps());
    }   

    gfx_destroy_sprite(&sprite0); 
    gfx_destroy(); 
    return 0;
}
