
//Atlas renderer

//To render
/*
    Raw Data is a R16 Window resolution is 240*4 160*4
    The fragment shader reads each r16 block as a texel. 
    RA_B_C_D  
    x,y     x+1,y
    ----------
    | A | B |
    ---------
    | C | D |
    x,y+1   x+1,y+1
    Where x,y is position in NB Screen space
    Each 4 bit values maps into the palette. 
    

*/
//Map x [A,B] to Y[C,D] 
//Y = (X-A)/(B-A) * (D-C) + C
#include "nb.h"

//
static const char * uniform_scale   ="scale";
static const char * uniform_width   ="width";
static const char * uniform_height   ="height";

//Maps -1.1 to width height
static const char * vertex_source = 
    "#version 130\n"
    "uniform float width;"
    "uniform float height;"
    "in vec2 in_pos;"
    "in vec2 in_uv;"
    "out vec2 uv;"
    "void main(){"
        "uv=in_uv;"
        "gl_Position =vec4(vec2( in_pos.x/width*2-1, -1*in_pos.y/height*2+1),1,1);\n"
    "}";

const char * fragment_source = 
    "#version 130\n"
    "uniform sampler2D sampler;"
    "in vec2 uv;"
    "out vec4 color;"
    "void main(){\n"
        "color = texture(sampler,uv);\n"
        //"color =vec4(uv,1,1);\n"
    "}";

void nb_draw_screen(nb_cpu * cpu){}


int demo()
{
    nb_cpu cpu;

    int width = NB_SCREEN_WIDTH, height = NB_SCREEN_HEIGHT;
    gfx_init(NB_TITLE, width, height);
    gfx_load_shader(vertex_source, fragment_source);
    gfx_mesh palette_mesh;
    gfx_vertex palette_verts[6] = {
        //pos,  uv
        { {0,                   0                   },  {0,0}},
        { {0,                   NB_SCREEN_HEIGHT/2  },  {0,1}},
        { {NB_SCREEN_WIDTH/2,   NB_SCREEN_HEIGHT/2  },  {1,1}},        
        { {0,                   0                   },  {0,0}},
        { {NB_SCREEN_WIDTH/2,   0                   },  {1,0}},
        { {NB_SCREEN_WIDTH/2,   NB_SCREEN_HEIGHT/2  },  {1,1}}
    };

    gfx_load_mesh(   &palette_mesh, palette_verts,  6);
    
    //each palette color is a R8G8B8A8
    
    color  * palette_data  = (color*)malloc(NB_PALETTE_SIZE * sizeof(color));
    color * c;
    int i,j,x;
    //for rendering
    int palette_height = 2;
    int palette_width = NB_PALETTE_SIZE/palette_height;
    int n=0;
    
    for(j=0; j < palette_height; j++)
        for(i=0; i < palette_width; i++)
        {
            x = ( (j*palette_width)+i);
            
            c = &palette_data[x];
            c->r = (x*sizeof(color))%255;
            c->g = 0;
            c->b = 0;
            c->a = 1;
        }
    gfx_texture palette_texture;
    
    gfx_load_texture(   &palette_texture, 
                        GFX_TEXTURE_2D, GFX_RGBA8,
                        palette_data,
                        palette_width, palette_height); 

    //set the current palette to the cpu palette
    cpu.palette = &(palette_data[0]);
    
    while(gfx_update())
    {
        //if dirty
        gfx_update_texture(  &palette_texture,
                            0,
                            0, 
                            palette_width, 
                            palette_height
                        ); 
        
        //draw
        gfx_set_uniform(uniform_width, width);
        gfx_set_uniform(uniform_height, height);
        gfx_bind_texture(&palette_texture);
        gfx_render(&palette_mesh);
    }   

    free(palette_data);
    gfx_destroy_mesh(&palette_mesh); 
    gfx_destroy(); 
    return 0;
}


