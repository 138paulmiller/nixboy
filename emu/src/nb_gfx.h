#ifndef GFX
#define GFX
#include <stdio.h>
#include <stdlib.h>
#include "nb_types.h"

#define GFX_ATTRIB_POS  "in_pos"
#define GFX_ATTRIB_UV   "in_uv"

typedef enum gfx_format
{
    GFX_R8,
    GFX_RG8,
    GFX_RGB8,
    GFX_RGBA8 
} gfx_format;

typedef enum gfx_type
{
    GFX_TEX_1D,
    GFX_TEX_2D 
} gfx_type;


typedef struct gfx_texture
{
    int handle;          //texture object handle
    int width, height;  //Dimension of texture data
    byte * data;        //weak, nonowning ref to pixel data
    //correspond to gfx_type and gfx_format but contain the GL  equivalent
    int  type;     
    int format;   
} gfx_texture ;

typedef struct gfx_vertex
{
    float pos[2]; //weak no owning vertex data ptr. Change to byte  scale to console size
    float uv[2]; //weak no owning vertex data ptr. Change to byte  scale to console size
} gfx_vertex;

typedef struct gfx_mesh
{
    int vao, vbo; //handles : vertex array obj, vertex buffer object
    gfx_vertex * verts; //weak no owning vertex data ptr. Change to byte  scale to console size
    int num_verts;

} gfx_mesh ;

void    gfx_init(const char * title, int width, int height);

void    gfx_destroy();

void    gfx_clear();

int     gfx_update();

void    gfx_load_shader(const char * vertex_source, const char * fragment_source);

//todo specifiy render targetsvoid gfx_render();

void    gfx_load_mesh(gfx_mesh * mesh, gfx_vertex * verts, int size);

void    gfx_render(gfx_mesh * mesh);

void    gfx_destroy_mesh(gfx_mesh * mesh);

void    gfx_bind_texture(gfx_texture * texture );

void    gfx_load_texture(gfx_texture * texture, gfx_type type, gfx_format format, byte  * data, int width, int height);

    //x,y,w,h, is sub rect
void    gfx_update_texture(gfx_texture * texture, int x, int y, int width, int height);

void    gfx_set_uniform(const char * name, float value);

#endif