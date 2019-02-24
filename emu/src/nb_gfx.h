#ifndef GFX
#define GFX
/*
Tile Renderer
*/

#include "nb_types.h"
//Expected defs
#define GFX_ATTRIB_VERT         "in_vert"
#define GFX_ATTRIB_UV           "in_uv"
#define GFX_UNIFORM_POS         "pos"
#define GFX_UNIFORM_SIZE        "size"
#define GFX_UNIFORM_SCALE       "scale"
#define GFX_UNIFORM_RESOLUTION  "resolution"
#define GFX_UNIFORM_ATLAS       "atlas"
#define GFX_UNIFORM_PALETTE     "palette"
//atlas offset for sprites/tiles 
#define GFX_UNIFORM_OFFSET     "offset"
#define GFX_UNIFORM_PALETTE_SIZE "palette_size"
#define GFX_UNIFORM_SPRITE_ATLAS_SIZE "sprite_atlas_size"
#define GFX_UNIFORM_COLOR_DEPTH "color_depth"

#define GFX_PALETTE_TEXTURE_UNIT    0
#define GFX_ATLAS_TEXTURE_UNIT      1

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
    GFX_SPRITE_REGULAR,//Regular 8x8
    GFX_SPRITE_WIDE,//Tall 8x16
    GFX_SPRITE_TALL //Wide 18x8
} gfx_sprite_type;

typedef struct gfx_shader
{
    u32 vert_shader;
    u32 frag_shader;
    u32 program;
    //vbo locations
    u32 vert_loc;
    u32 uv_loc;
    //
} gfx_shader;

typedef struct gfx_texture
{
    u32 handle;          //texture object handle
    u32 width, height;  //Dimension of texture data
    byte * data;        //weak, nonowning ref to pixel data
    //correspond to gfx_type and gfx_format but contain the GL  equivalent
    u32  type;     
    u32 format;   
} gfx_texture ;

typedef struct gfx_vertex
{
    vec2f pos;   //weak no owning vertex data ptr. Change to byte  scale to console size
    vec2b uv;    //weak no owning vertex data ptr. Change to byte  scale to console size
} gfx_vertex;

//
typedef struct gfx_mesh
{
    u32 vao, vbo; //handles : vertex array obj, vertex buffer object
    gfx_vertex * verts; //weak no owning vertex data ptr. Change to byte  scale to console size
    gfx_shader * shader;
    u32 num_verts;

} gfx_mesh ;

//Allocates and frees mesh and vertices!
//Binds 
typedef struct gfx_rect
{
    vec2f pos;                  //position in level
    vec2f size;                 //size to render
    gfx_mesh  mesh;         //texture to render
} gfx_rect;


typedef struct gfx_palette
{
    rgb * data;           //nonowning ptr
    gfx_texture texture;    //atlas sheet (palette indices)
} gfx_palette;


typedef struct gfx_atlas
{       
    byte * indices;         //nonowning ptr
    gfx_texture texture;    //atlas sheet (palette indices)
} gfx_atlas;


//A Sheet representa a texture whose indices map to palette colors
typedef struct gfx_sheet
{
    gfx_palette * palette;
    gfx_atlas * atlas;    //atlas sheet (palette indices)

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
    gfx_sheet * sheet;      //nonwoning source sprite sheet ref
    vec2i offset;           //sheet offset (top left corner to start reading from )
    gfx_sprite_type type;   //size of rect to to read from 
    //sprites are tiles whose positions are not determined by level map, but an offset
    gfx_rect rect; //size determined by type
    bool flip_x; //if flipped along y axis 
    bool flip_y; //if flipped along y axis 
} gfx_sprite;

typedef struct gfx_timer
{
    u32 now_ticks;   //time since paused
    u32 last_ticks;   //time since paused
    u32 delta_ticks;     //delta time in ms
} gfx_timer;

//gfx_nit* does not alloc objects!

// --------- GFX Core ----- 
gfx_status  gfx_init(const char * title, int width, int height);
void        gfx_destroy();
void        gfx_clear();
int         gfx_update();

// FPS Utils

void        gfx_cap_fps(int max_fps);
void        gfx_uncap_fps();
float       gfx_delta_sec();
float       gfx_delta_ms();
float       gfx_fps();  //
float       gfx_fpms();  //framesper millisec

// ---------- GFX Timer ------------ updated on gfx_init and gfx_update
void        gfx_timer_init(gfx_timer * timer);
void        gfx_timer_tick(gfx_timer * timer);   //elapsed time while paused or not

// --------- GFX Shader ----- 
gfx_status  gfx_init_shader(gfx_shader * shader, const char * vertex_source, const char * fragment_source);
void        gfx_destroy_shader(gfx_shader * shader);
void        gfx_bind_shader(gfx_shader * shader);
gfx_status  gfx_set_uniform_float(gfx_shader * shader, const char * name, float value);
gfx_status  gfx_set_uniform_int(gfx_shader * shader, const char * name, int value);
gfx_status  gfx_set_uniform_vec2i(gfx_shader * shader, const char * name, const vec2i * value);
gfx_status  gfx_set_uniform_vec2f(gfx_shader * shader, const char * name, const vec2f * value);

// --------- GFX Mesh ----- 
gfx_status  gfx_init_mesh(gfx_mesh * mesh,  gfx_shader *shader, gfx_vertex * verts, int num_verts);
void        gfx_destroy_mesh(gfx_mesh * mesh);
void        gfx_render_mesh(gfx_mesh * mesh);

// --------- GFX Texture ----- 
gfx_status  gfx_init_texture(u32 texture_location, gfx_texture * texture, gfx_texture_type type, gfx_format data_format, byte  * data, int width, int height);
void        gfx_bind_texture(u32 texture_location, gfx_texture * texture);
void        gfx_update_texture(u32 texture_location, gfx_texture * texture, int x, int y, int width, int height);

// --------- GFX Rect ----- 
gfx_status  gfx_init_rect(gfx_rect * rect, gfx_shader * shader, float x, float y, float w, float h);
void        gfx_destroy_rect(gfx_rect * rect);
void        gfx_render_rect(gfx_rect * rect);

// ------------ GFX Palette -----------------------
void        gfx_init_palette(gfx_palette *palette, rgb * data,u32 width, u32 height);
void        gfx_destroy_palette(gfx_palette *palette);
void        gfx_update_palette(gfx_palette *palette); //if data changes
void        gfx_use_palette(gfx_palette *palette );

// ------------ GFX atlas -----------------------
void        gfx_init_atlas(gfx_atlas * atlas, byte * indices,u32 width, u32 height);
void        gfx_destroy_atlas(gfx_atlas * atlas);
void        gfx_update_atlas(gfx_atlas *atlas); //if indices change
void        gfx_use_atlas(gfx_atlas * atlas);

// ------------ GFX Sheet -----------------------
void        gfx_init_sheet(gfx_sheet * sheet, gfx_palette * palette, gfx_atlas * atlas);
void        gfx_destroy_sheet(gfx_sheet * sheet);
void        gfx_use_sheet(gfx_sheet * sheet);

// --------- GFX Sprite  ----- 
gfx_status  gfx_init_sprite(gfx_sprite * sprite, gfx_sheet * sheet, gfx_shader * shader, vec2i offset, gfx_sprite_type type);
void        gfx_destroy_sprite(gfx_sprite * sprite);
void        gfx_render_sprite(gfx_sprite * rect);
void        gfx_get_sprite_xy(gfx_sprite * sprite, float * x, float * y );
void        gfx_set_sprite_xy(gfx_sprite * sprite, float  x, float  y );
void        gfx_flip_sprite(gfx_sprite * sprite, bool x_flip, bool y_flip );


#endif