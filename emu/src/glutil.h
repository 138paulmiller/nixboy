#ifndef GLUTIL
#define GLUTIL
#include <stdio.h>
#include <stdlib.h>
typedef unsigned  char byte ;
typedef unsigned  int uint ;

void gl_init(int width, int height);
void gl_destroy();
void gl_clear();
int gl_update();
void  gl_render();

//todo specifiy render targetsvoid gl_render();

int gl_test();


//TODO Remove and change from load_map, load_sprite, load_palette
void gl_load_texture(int * texture, byte * data, int x, int y, int w, int h,int comp);

void gl_update_texture(int texture, byte * data, int x, int y, int w, int h,int comp);
#endif