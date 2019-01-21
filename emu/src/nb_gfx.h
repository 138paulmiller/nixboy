#ifndef GFX
#define GFX
/*
Tile Renderer
*/

#include <stdio.h>
#include <stdlib.h>
#include "nb_gc.h"

#define GFX_ATTRIB_POS  "in_pos"
#define GFX_ATTRIB_UV   "in_uv"


typedef enum gfx_status
{
    GFX_FAILURE =0,
    GFX_SUCCESS =1,
    
} gfx_status;

typedef enum gfx_format
{
    GFX_R8,
    GFX_RG8,
    GFX_RGB8,
    GFX_RGBA8 
} gfx_format;

typedef enum gfx_texture_type
{
    GFX_TEXTURE_1D,
    GFX_TEXTURE_2D 
} gfx_texture_type;

typedef enum gfx_sprite_type
{
    GFX_SPRITE_REGULAR,
    GFX_SPRITE_WIDE,
    GFX_SPRITE_TALL 
} gfx_sprite_type;

typedef struct gfx_shader
{
    int vert_shader;
    int frag_shader;
    int program;
    //vbo locations
    int pos_loc;
    int uv_loc;

} gfx_shader;

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
    float pos[2];   //weak no owning vertex data ptr. Change to byte  scale to console size
    float uv[2];    //weak no owning vertex data ptr. Change to byte  scale to console size
} gfx_vertex;

typedef struct gfx_mesh
{
    int vao, vbo; //handles : vertex array obj, vertex buffer object
    gfx_vertex * verts; //weak no owning vertex data ptr. Change to byte  scale to console size
    gfx_shader * shader;    //shader that will draw the mesh. To change, reinit the mesh
    int num_verts;

} gfx_mesh ;

//Allocates and frees mesh and vertices!
//Binds 
typedef struct gfx_rect
{
    vec2i pos;                  //position in level
    vec2i size;                 //size to render
    gfx_mesh    mesh;         //texture to render
} gfx_rect;

//Atlases represent a texture whose indices map to palette colors
typedef struct gfx_palette
{
    gfx_texture * texture;
} gfx_palette;

//A Sheet representa a texture whose indices map to palette colors
typedef struct gfx_sheet
{
    gfx_palette * palette;
    gfx_texture * atlas;    //atlas sheet (palette indices)
} gfx_sheet;

typedef struct gfx_tilemap
{
    //each valie indexes into a region of the tilesheet. At offset
    gfx_texture * map;    //atlas sheet
} gfx_tilemap;

//At tile is a subrect of its sheet  
//Rendering sprite, binds palette, binds sheet, then renders rect  
typedef struct gfx_sprite
{
    gfx_sheet * sheet; //source sprite sheet
    vec2i offset;           //sheet offset (top left corner to start reading from )
    gfx_sprite_type type; //size to read from 
    //sprites are tiles whose positions are not determined by level map, but an offset
    gfx_rect rect; //size determined by type
    byte flip_x; //if flipped along y axis 
    byte flip_y; //if flipped along y axis 
} gfx_sprite;

//gfx_nit* does not alloc objects!

// --------- GFX Core ----- 
gfx_status  gfx_init(const char * title, int width, int height);
void        gfx_destroy();
void        gfx_clear();
int         gfx_update();

// --------- GFX Shader ----- 
gfx_status  gfx_init_shader(gfx_shader * shader, const char * vertex_source, const char * fragment_source);
void        gfx_destroy_shader(gfx_shader * shader);
void        gfx_bind_shader(gfx_shader * shader);
gfx_status  gfx_set_uniform(gfx_shader * shader, const char * name, float value);

// --------- GFX Mesh ----- 
gfx_status  gfx_init_mesh(gfx_mesh * mesh, gfx_shader * shader, gfx_vertex * verts, int num_verts);
void        gfx_destroy_mesh(gfx_mesh * mesh);
void        gfx_render_mesh(gfx_mesh * mesh);

// --------- GFX Texture ----- 
gfx_status  gfx_init_texture(gfx_texture * texture, gfx_texture_type type, gfx_format format, byte  * data, int width, int height);
void        gfx_bind_texture(gfx_texture * texture );
void        gfx_update_texture(gfx_texture * texture, int x, int y, int width, int height);

// --------- GFX Rect ----- 
gfx_status  gfx_init_rect(gfx_rect * rect, gfx_shader * shader, int x, int y, int w, int h);
void        gfx_destroy_rect(gfx_rect * rect);
void        gfx_render_rect(gfx_rect * rect);

#endif