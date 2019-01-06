#include "glutil.h"

static const char * uniform_scale="scale";
static const char * title="nixboy";

//Tile based renderer

typedef struct Memory
{
    byte * palette; // 256 24 bit  Color values 
    byte * tiles;   //tile sheet 8 bit 8x8 240x160 color grids  
    byte * map;     //tile map  240x160 tile indexes
}Memory ;


int begin()
{
    Memory memory;

    int width = 256, height = 256;
    gl_init(title, width, height);
    
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
    int palette_size = palette_width * palette_height * palette_comp;
    memory.palette = (byte*)malloc(palette_size );
    byte * c;
    int i,j,x;
    for(j=0; j < palette_height; j++)
        for(i=0; i < palette_width; i++)
        {
            x = ( (j*palette_height)+i) * palette_comp;
            memory.palette[x]=x%255;
            memory.palette[x+1]=0;
            memory.palette[x+2]=0;
        }

    texture palette_texture;
    
    gl_load_texture(    &palette_texture, 
                        memory.palette,
                        palette_width, 
                        palette_height, 
                        palette_comp
                    ); 

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
    gl_mesh_destroy(&palette_mesh); 
    gl_destroy(); 
    return 0;
}



int main()
{
    begin();
}
