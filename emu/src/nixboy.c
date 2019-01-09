#include "glutil.h"

static const char * uniform_scale="scale";
static const char * title="nixboy";

//Tile based renderer

static const char * vertex_source = 
    "#version 130\n"
    //"uniform vec2 size;"
    "in vec2 pos;"
    "out vec2 uv;"
    "void main(){"
        "uv=pos;"
        "gl_Position =vec4(uv,1,1);\n"

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



typedef struct Memory
{
    //All references are non-owning to allow swapping 
    byte * palette; // 256 24 bit  Color values 
    byte * tiles;   //tile sheet 8 bit 8x8 240x160 color grids  
    byte * map;     //tile map  240x160 tile indexes
}Memory ;


int begin()
{
    Memory memory;

    int width = 256, height = 256;
    gl_init(title, width, height);
    gl_load_shader(vertex_source, fragment_source);
    mesh palette_mesh;
    float palette_verts[12] = {
        0, 0,
        0, 1,
        1, 1,

        0, 0,
        1, 0,
        1, 1,
    };
    gl_load_mesh(   &palette_mesh, 
                    palette_verts,  
                    sizeof(palette_verts), 
                    2);
    
    //each palette color is a R8G8B8A8
    int palette_width   = 16, 
        palette_height  = 16, 
        palette_comp    = 3;
    int palette_size = palette_width * palette_height;
    
    color * palette_data  = (color*)malloc(palette_size *  sizeof(color) );
    color * c;
    int i,j,x;
    for(j=0; j < palette_height; j++)
        for(i=0; i < palette_width; i++)
        {
            x = ( (j*palette_height)+i);
            c = &palette_data[x];
            c->r =x%256;
            c->g=i;
            c->b=j;
        }

    texture palette_texture;
    
    gl_load_texture(    &palette_texture, 
                        &palette_data[0],
                        palette_width, 
                        palette_height, 
                        palette_comp
                    ); 

    //set the current palette to the cpu palette
//memory.palette = &palette_data[0].data[0];
    
    while(gl_update())
    {
        //if dirty
        
        gl_update_texture(  &palette_texture,
                            0,
                            0, 
                            palette_width, 
                            palette_height
                        ); 
        //draw
        //gl_set_uniform(uniform_scale, scale);
        gl_bind_texture(&palette_texture);
        gl_render(&palette_mesh);
    }   

    free(palette_data);
    gl_mesh_destroy(&palette_mesh); 
    gl_destroy(); 
    return 0;
}



int main()
{
    begin();
}
