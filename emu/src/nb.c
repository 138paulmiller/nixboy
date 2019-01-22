
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
static const char * uniform_size   ="size";
static const char * uniform_pos   ="pos";
static const char * uniform_resolution   ="resolution";

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
        "uv=in_uv;"
        "vec2 new_pos = (size*in_vert+pos)/resolution*2.0-1.0;"
      //  "vec2 new_pos = vec2( in_vert.x/resolution.x*2.0-1.0, -1*in_vert.y/resolution.y*2.0+1.0);"
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


int demo()
{
    nb_cpu cpu;
    vec2i resolution = { NB_SCREEN_WIDTH, NB_SCREEN_HEIGHT};
    int width = NB_SCREEN_WIDTH, height = NB_SCREEN_HEIGHT;
    gfx_init(NB_TITLE, width*2, height*2);
    
    gfx_shader shader;
    gfx_init_shader(&shader, vertex_source, fragment_source);
    gfx_mesh palette_mesh;
/*
    gfx_vertex palette_verts[6] = {
        //pos,  uv
        { {0,   0   },  {0,0}},
        { {0,   1   },  {0,1}},
        { {1,   1   },  {1,1}},        
        { {0,   0   },  {0,0}},
        { {1,   0   },  {1,0}},
        { {1,   1   },  {1,1}}
    };
    gfx_vertex palette_verts[4] = {
        //pos,  uv
        { {0,                   0                   },  {0,0}},
        { {0,                   NB_SCREEN_HEIGHT/2  },  {0,1}},
        { {NB_SCREEN_WIDTH/2,   NB_SCREEN_HEIGHT/2  },  {1,1}},
        { {NB_SCREEN_WIDTH/2,   0                   },  {1,0}},
    };

    //init mesh with the current shader. Mesh will be drawn with this shader
    if(gfx_init_mesh(   &palette_mesh, &shader, palette_verts,  sizeof(palette_verts)/sizeof(palette_verts[0])) == GFX_FAILURE)
    {
        nb_error("Failed to init mesh");
    }
*/
    gfx_rect palette_rect;

    gfx_init_rect(&palette_rect, &shader, 0,0,NB_SCREEN_WIDTH/2,NB_SCREEN_HEIGHT/2);
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

    //set the current palette to the cpu palette
    cpu.palette = (byte*)palette_data;

    gfx_cap_fps(60);
    while(gfx_update())
    {

        //physics step (delta)
        palette_rect.pos.x += gfx_delta_sec()*16; 
        //draw        
        gfx_bind_shader(&shader);
        gfx_set_uniform_vec2i(&shader, uniform_resolution, &resolution);
        
        //if dirty
        gfx_update_texture(  &palette_texture,0,0, palette_width, palette_height); 
        gfx_bind_texture(&palette_texture);

        gfx_render_rect(&palette_rect);
        nb_log("FPS\t%.03f\n", gfx_fps());
    }   

    gfx_destroy_rect(&palette_rect); 
    gfx_destroy(); 
    return 0;
}


