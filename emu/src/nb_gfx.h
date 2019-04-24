#ifndef GFX
#define GFX
/*
gfx - nixboy grapics module

Tile based sprite renderer

*/

#include "nb_defs.h"
#include "nb_keys.h"

//Expected defs
#define NB_ATTRIB_VERT                 "in_vert"
#define NB_ATTRIB_UV                   "in_uv"
#define NB_UNIFORM_POS                 "pos"
#define NB_UNIFORM_SIZE                "size"
#define NB_UNIFORM_SCREEN_SCALE        "screen_scale"
#define NB_UNIFORM_SCREEN_RESOLUTION   "screen_resolution"
#define NB_UNIFORM_ATLAS               "atlas"
#define NB_UNIFORM_PALETTE             "palette"
//atlas offset for sprites/tiles 

#define NB_UNIFORM_OFFSET              "offset"
#define NB_UNIFORM_PALETTE_SIZE        "palette_size"
#define NB_UNIFORM_ATLAS_RESOLUTION    "atlas_resolution"
#define NB_UNIFORM_COLOR_DEPTH         "color_depth"

#define NB_TEXTURE_UNIT_PALETTE     0
#define NB_TEXTURE_UNIT_ATLAS      1
#define NB_TEXTURE_UNIT_TILEMAP     2   //R8 texture used for lookups into tile atlas  


// 
typedef struct nb_mouse
{   
    u16 x, y;

} nb_mouse;

typedef struct nb_timer
{
    u32 now_ticks;   //time since paused
    u32 last_ticks;   //time since paused
    u32 delta_ticks;     //delta time in ms
} nb_timer;


typedef enum nb_format
{
    NB_R8,
    NB_RG8,
    NB_RGB8,
    NB_RGBA8 
} nb_format;

typedef enum nb_texture_type
{
    NB_TEXTURE_1D,
    NB_TEXTURE_2D  
} nb_texture_type;

typedef enum nb_sprite_type
{
    NB_SPRITE_REGULAR,//Regular 8x8
    NB_SPRITE_WIDE,//Tall 8x16
    NB_SPRITE_TALL //Wide 18x8
} nb_sprite_type;

typedef struct nb_shader
{
    u32 vert_shader;
    u32 frag_shader;
    u32 program;

    //vbo locations
    u32 vert_loc;
    u32 uv_loc;
    //
} nb_shader;

typedef struct nb_texture
{
    u32 width, height;  //Dimension of texture data
    byte * data;        //weak, nonowning ref to pixel data
    //correspond to nb_type and nb_format but contain the GL  equivalent
    // gl specific vars
    u32 handle;          //texture object handle
    u32  type;     
    u32 format, internal_format;
} nb_texture ;

typedef struct nb_vertex
{
    vec2f pos;   //weak no owning vertex data ptr. Change to byte  scale to console size
    vec2b uv;    //weak no owning vertex data ptr. Change to byte  scale to console size
} nb_vertex;

//
typedef struct nb_mesh
{
    u32 vao, vbo; //handles : vertex array obj, vertex buffer object
    nb_vertex * verts; //weak no owning vertex data ptr. Change to byte  scale to console size
    nb_shader * shader;
    u32 num_verts;

} nb_mesh ;

//Allocates and frees mesh and vertices!
//Binds 
typedef struct nb_rect
{
    vec2f pos;                  //position in level
    vec2f size;                 //size to render
    nb_mesh  mesh;         //texture to render
} nb_rect;


typedef struct nb_palette
{
    rgb * data;           //nonowning ptr
    nb_texture texture;    //atlas sheet txture (palette indices)
} nb_palette;


typedef struct nb_atlas
{       
    byte * indices;         //nonowning ptr
    nb_texture texture;    //atlas sheet texture (palette indices)
} nb_atlas;


//At tile is a subrect of its sheet  
//Rendering sprite, binds palette, binds sheet, then renders rect  
typedef struct nb_sprite
{
    
    vec2i offset;           //sheet offset (top left corner to start reading from )
    //sprites are tiles whose positions are not determined by level map, but an offset
    nb_rect rect; //size determined by type
    bool is_active;
    bool flip_x; //if flipped along y axis 
    bool flip_y; //if flipped along y axis 
} nb_sprite;


//Rendering level, binds palette, and tile atlas  
typedef struct nb_level
{
    vec2i offset;           
    nb_atlas * tile_atlas;
} nb_level; //TODO




//nb_nit* does not alloc objects!

// --------- GFX Core ----- 
nb_status  nb_init_window(const char * title, int width, int height);
void       nb_destroy_window();
void       nb_clear_window();
nb_status  nb_update_window();
void       nb_get_mouse_xy(u16 * x, u16 * y);


// FPS Utils
void        nb_cap_fps(int max_fps);
void        nb_uncap_fps();
float       nb_delta_sec();
float       nb_delta_ms();
float       nb_fps();  //
float       nb_fpms();  //framesper millisec


// ---------- GFX Timer ------------ updated on nb_init and nb_update

void        nb_timer_init(nb_timer * timer);
void        nb_timer_tick(nb_timer * timer);   //elapsed time while paused or not

// --------- GFX Shader ----- 
nb_status  nb_init_shader( nb_shader * shader, 
                            const char * vertex_source, 
                            const char * fragment_source ) ;
void        nb_destroy_shader( nb_shader * shader    );
void        nb_bind_shader   ( nb_shader * shader    );

//uniform setters 

nb_status  nb_set_uniform_float( nb_shader * shader,
                                   const char * name, 
                                   float value         );

nb_status  nb_set_uniform_int   ( nb_shader * shader, 
                                    const char * name, 
                                    int value);

nb_status  nb_set_uniform_vec2i ( nb_shader * shader, 
                                    const char * name, 
                                    const vec2i * value);

nb_status  nb_set_uniform_vec2f ( nb_shader * shader, 
                                    const char * name, 
                                    const vec2f * value);

// --------- GFX Mesh ----- 
nb_status  nb_init_mesh    ( nb_mesh * mesh,  
                               nb_shader *shader, 
                               nb_vertex * verts, 
                               int num_verts     );

void        nb_destroy_mesh ( nb_mesh * mesh);

void        nb_render_mesh  ( nb_mesh * mesh);

// --------- GFX Texture ----- 
nb_status  nb_init_texture (  u32 texture_location, 
                                nb_texture * texture, 
                                nb_texture_type type,
                                nb_format data_format, 
                                byte  * data, 
                                int width, int height  );

void        nb_bind_texture (  u32 texture_location, 
                                nb_texture * texture);

void        nb_update_texture( u32 texture_location, 
                                nb_texture * texture, 
                                int x, int y, 
                                int width, int height);

void nb_destroy_texture      ( nb_texture * texture);
// --------- GFX Rect ----- 
nb_status  nb_init_rect(nb_rect * rect, nb_shader * shader, float x, float y, float w, float h);

void        nb_destroy_rect(nb_rect * rect);

void        nb_render_rect(nb_rect * rect);

// ------------ GFX Palette -----------------------
void        nb_init_palette   (nb_palette *palette, 
                                rgb * data,
                                u32 width);

void        nb_destroy_palette(nb_palette *palette);

void        nb_update_palette (nb_palette *palette); //if data changes

void        nb_use_palette    (nb_palette *palette );

// ------------ GFX atlas -----------------------
void        nb_init_atlas     (nb_atlas * atlas, 
                                byte * indices,
                                u32 width, 
                                u32 height);
void        nb_destroy_atlas  (nb_atlas * atlas );
void        nb_update_atlas   (nb_atlas *atlas); //if indices change
void        nb_use_atlas      (nb_atlas * atlas);


// --------- GFX Sprite  ----- 

nb_status  nb_init_sprite   ( nb_sprite * sprite,
                                nb_shader * shader, 
                                vec2i offset, vec2f size ) ;

void        nb_destroy_sprite( nb_sprite * sprite  ) ;

void        nb_render_sprite ( nb_sprite * sprite  ) ;

void        nb_get_sprite_xy ( nb_sprite * sprite,  
                                float * x,   
                                float * y   ) ;

void        nb_set_sprite_xy ( nb_sprite * sprite,
                                float  x,   
                                float  y    ) ;

void        nb_flip_sprite   (nb_sprite * sprite, 
                                bool x_flip, 
                                bool y_flip  ) ;


#endif