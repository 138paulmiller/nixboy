#ifndef GLUTIL
#define GLUTIL
#include <stdio.h>
#include <stdlib.h>
typedef unsigned char byte ;
typedef unsigned int uint ;

typedef struct color
{
    union
    {
        struct {byte r,g,b;};
        byte data[3];
    };
        
}color;


//drawable rentangle
typedef struct 
{
    int handle;        //texture object handle 
    color * data;        //weak, nonowning ref
    int width, height;
    int comp;

} texture ;

typedef struct 
{
    int vao, vbo; //handles : vertex array obj, vertex buffer object
    float * data; //weak no owning data ptr
    int size, num_verts;

} mesh ;

void gl_init(const char * title, int width, int height);
void gl_destroy();
void gl_clear();
int gl_update();


void gl_load_shader(const char * vertex_source, const char * fragment_source);

//todo specifiy render targetsvoid gl_render();

void gl_load_mesh(mesh * obj, float * data, float size, float comp);

void gl_render(mesh * obj);

void gl_mesh_destroy(mesh * obj);

//TODO Remove and change from load_map, load_sprite, load_palette
void gl_bind_texture(texture * obj);

void gl_load_texture(texture * obj, color * data,  int width, int height,int comp);

//x,y,w,h, is sub rect
void gl_update_texture(texture * obj, int x, int y, int width, int height);

void gl_set_uniform(const char * name, float value);

#endif