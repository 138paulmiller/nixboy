#include "glutil.h"

const char * uniform_scale="scale";

//Tile based renderer

typedef struct color
{
    union
    {
        struct
        {
            byte r,g,b;
        };
        byte * data;
    };
}color;

typedef struct Memory
{
    byte * palette; // 256 24 bit  Color values 
    byte * tiles;   //tile sheet 8 bit 8x8 240x160 color grids  
    byte * map;     //tile map  240x160 tile indexes
}Memory ;


int begin()
{
    Memory memory;
    //TODO
    int scale = 1;
    int width = scale*256, height = scale*256;
    gl_init(width, height);
    
    mesh palette_mesh;
    float palette_verts[12] = {
        0, 0,
        0, 1,
        1, 1,

        0, 0,
        1, 0,
        1, 1,
    };
    gl_load_mesh(&palette_mesh, palette_verts,  sizeof(palette_verts)*sizeof(float), 2);
    
    int palette_width = 128, palette_height = 128, palette_comp = 3;
    memory.palette = (byte*)malloc(palette_width * palette_height * palette_comp);
    
    int i,j,x; 
    for(j=0; j < palette_height; j++)
        for(i=0; i < palette_width; i++)
        {
            x = palette_comp*(j*palette_height+i);
            memory.palette[x++] =  i;
            memory.palette[x++] =  0;
            memory.palette[x  ] =  0;
        }

    texture palette_texture;
    
    gl_load_texture(&palette_texture, memory.palette,palette_width, palette_height, palette_comp); 

    while(gl_update())
    {
        //if dirty
        gl_update_texture(&palette_texture,0,0, palette_width, palette_height); 
        //draw
        //gl_set_uniform(uniform_scale, scale);
        gl_bind_texture(&palette_texture);
        gl_render(&palette_mesh);
    }   
    gl_destroy(); 
    return 0;
}



int main()
{
    begin();
}
