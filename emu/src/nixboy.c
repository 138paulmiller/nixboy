
#include "nixboy.h"

//
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

int begin()
{
    nb_cartridge cartridge;

    int width = 256, height = 256;
    gfx_init(title, width, height);
    gfx_load_shader(vertex_source, fragment_source);
    gfx_mesh palette_mesh;
    float palette_verts[12] = {
        0, 0,
        0, 1,
        1, 1,

        0, 0,
        1, 0,
        1, 1,
    };
    gfx_load_mesh(   &palette_mesh, 
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
    gfx_texture palette_texture;
    
    gfx_load_texture(    &palette_texture, 
                        &(palette_data[0].data[0]),
                        palette_width, 
                        palette_height, 
                        palette_comp
                    ); 

    
    //set the current palette to the cpu palette
    cartridge.palette = &(palette_data[0].data[0]);
    
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
        //gfx_set_uniform(uniform_scale, scale);
        gfx_bind_texture(&palette_texture);
        gfx_render(&palette_mesh);
    }   

    free(palette_data);
    gfx_destroy_mesh(&palette_mesh); 
    gfx_destroy(); 
    return 0;
}



int main()
{
    begin();
}
