#ifndef GFX
#define GFX
#include <stdio.h>
#include <stdlib.h>
#include "nb_types.h"


//TExture represents a 3 component image
typedef struct gfx_texture
{
    int handle;          //texture object handle
    int format;         // component format of data, r8g8b8 for image, r16 for lookup
    int width, height;
    byte * data;          //weak, nonowning ref to pixel data
} gfx_texture ;


typedef struct gfx_mesh
{
    int vao, vbo; //handles : vertex array obj, vertex buffer object
    float * data; //weak no owning vertex data ptr. Change to byte  scale to console size
    int size, num_verts;

} gfx_mesh ;

void    gfx_init(const char * title, int width, int height);
void    gfx_destroy();
void    gfx_clear();
int     gfx_update();


void    gfx_load_shader(const char * vertex_source, const char * fragment_source);

//todo specifiy render targetsvoid gfx_render();

void    gfx_load_mesh(gfx_mesh * mesh, float * data, float size, float comp);

void    gfx_render(gfx_mesh * mesh);

void    gfx_destroy_mesh(gfx_mesh * mesh);

void    gfx_bind_texture(gfx_texture * texture );

void    gfx_load_texture(gfx_texture * texture, byte  * data, int width, int height, int comp );

    //x,y,w,h, is sub rect
void    gfx_update_texture(gfx_texture * texture, int x, int y, int width, int height);

void    gfx_set_uniform(const char * name, float value);

#endif