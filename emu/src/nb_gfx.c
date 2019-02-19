#include "nb_gfx.h"
#include "nb_gc.h"
#include "nb.h"

#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <SDL2/SDL.h>

//Global State
static SDL_Window*     _sdl_window;
static SDL_Event*      _sdl_event;
static SDL_GLContext   _sdl_gfx_context;
static gfx_shader *     _active_shader;  //create a shader stack to push and pop?
static gfx_sheet *     _active_sheet;  //create a shader stack to push and pop?
static gfx_timer        _fps_timer;  //create a shader stack to push and pop?

//Frames per Sec, Frames Per MilliSec
static int  _fps_cap  = -1;
static int _fpms_cap  = -1;

//map all indices from 1d to 2d using size of palette, sheet or map
// x = i/w
// y = i%w

static gfx_status _check_shader_error(int shader, int  flag, int is_program)
{
    int status = 0;
    if(is_program){
        glGetProgramiv(shader, flag, &status);
    }
    else{
        glGetShaderiv(shader, flag, &status);
    }
    if (status == GL_FALSE)
    {
        int error_len = 0;
        if(is_program ){
            glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &error_len);
        }
        else{
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &error_len);
        }
        char* error_msg = (char*)nb_malloc(error_len);
        if(is_program){
            glGetProgramInfoLog(shader, error_len, &error_len, error_msg);
        }
        else{
            glGetShaderInfoLog(shader, error_len, &error_len, error_msg);
        }
        printf("\nShader Log: %s \n", error_msg);
        nb_free( error_msg);
        return GFX_FAILURE;
    }
    return GFX_SUCCESS;
}

static  gfx_status _create_shader_stage(uint type, const char* source)
{
    int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, 0);
    glCompileShader(shader);
    
    if(_check_shader_error(shader, GL_COMPILE_STATUS,0)==GFX_FAILURE){
        return GFX_FAILURE;
    }
    
    return shader;

}


/////////////////////////////////////// Begin API ///////////////////////////////////////////////////


 gfx_status gfx_init(const char * title, int width, int height)
{
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0){
        puts("SDL Failed to initialize!"); exit(0);     
    }        
    
    _sdl_window = SDL_CreateWindow(title,SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    width, height, SDL_WINDOW_OPENGL);
    //Initialize opengl gfx_color attributes buffer size
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 2);

    _sdl_gfx_context = SDL_GL_CreateContext(_sdl_window);
    int status = glewInit(); 
    if(status != GLEW_OK)
    {
        nb_error("GLEW Failed to initialize %d", status);
        return GFX_FAILURE; 
    }   

    _active_shader = 0;
    _active_sheet = 0;
    gfx_timer_init(&_fps_timer);
    return GFX_SUCCESS;

}


void gfx_destroy()
{
    SDL_DestroyWindow(_sdl_window);
    SDL_GL_DeleteContext(_sdl_gfx_context);
    SDL_Quit();
}

void gfx_clear()
{
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


int gfx_update()
{
    //Poll input events
    SDL_Event event;
    while (SDL_PollEvent(&event))
    //TODO create Keymap 
        if (event.type == SDL_QUIT) return 0;

    SDL_GL_SwapWindow(_sdl_window);
    gfx_clear();
    gfx_timer_tick(&_fps_timer); 
    if(_fps_cap != -1 && gfx_fpms() < _fpms_cap)
    {
        SDL_Delay(( _fpms_cap - gfx_fpms() ));
    }
    return 1;
}


void gfx_cap_fps(int max_fps)
{
    _fps_cap  =max_fps;
    _fpms_cap  = 1000/_fps_cap;
}

void gfx_uncap_fps()
{
     _fps_cap  =_fpms_cap = -1;
}

// ---------------- FPS Utils-------------------------------------
float  gfx_delta_sec()
{
    return 0.001 * gfx_delta_ms();
}
float  gfx_delta_ms()
{
    return (_fps_timer.delta_ticks * 1000.f) /(float)SDL_GetPerformanceFrequency() ;
}
float  gfx_fps()
{
    return 1.0f/ (gfx_delta_sec());
}

float  gfx_fpms()
{
    return 1.0f/ (gfx_delta_ms());
}

void  gfx_timer_init(gfx_timer * timer){
    timer->delta_ticks = 0;
    timer->now_ticks =0;    
    timer->last_ticks = 0;
}

void gfx_timer_tick(gfx_timer * timer)
{

    timer->last_ticks = timer->now_ticks;
    timer->now_ticks = SDL_GetPerformanceCounter();
    timer->delta_ticks = (timer->now_ticks - timer->last_ticks);
}   //elapsed time while paused or not



// -------------------------- Shader ---------------------------------------
 gfx_status  gfx_init_shader(gfx_shader * shader, const char * vertex_source, const char * fragment_source)
{
    shader->program = glCreateProgram();
    shader->vert_shader   = _create_shader_stage(GL_VERTEX_SHADER, vertex_source);
    if(shader->vert_shader == GFX_FAILURE)
    {
        nb_error("Failed to Init Vertex Shader");
        return GFX_FAILURE;
    }
    shader->frag_shader   = _create_shader_stage(GL_FRAGMENT_SHADER,fragment_source);
    if(shader->frag_shader == GFX_FAILURE)
    {
        nb_error("Failed to Init Fragment Shader");
        return GFX_FAILURE;
    }
    
    glAttachShader(shader->program,  shader->vert_shader);
    glAttachShader(shader->program,  shader->frag_shader);
    glLinkProgram(shader->program);
    if(_check_shader_error(shader->program, GL_LINK_STATUS, 1)== GFX_FAILURE){
        return GFX_FAILURE;
    }

    
    shader->vert_loc = glGetAttribLocation(shader->program, GFX_ATTRIB_VERT);
    shader->uv_loc = glGetAttribLocation(shader->program, GFX_ATTRIB_UV);
    if(shader->vert_loc == -1)
    {
        nb_error("GFX Invalid name for Position Attrib: Expected %s", GFX_ATTRIB_VERT);
        return GFX_FAILURE;
    }
    if(shader->uv_loc == -1)
    {
        nb_error("GFX Invalid name for UV Attrib: Expected %s", GFX_ATTRIB_UV);
        return GFX_FAILURE;
    }


    return GFX_SUCCESS;

}
void    gfx_destroy_shader(gfx_shader * shader)
{
    glDeleteShader(shader->vert_shader); // removes
    glDeleteShader(shader->frag_shader); // removes
    glDeleteProgram(shader->program);

}

//bind only if not already bound
void    gfx_bind_shader(gfx_shader * shader)
{
    if(_active_shader != shader)
    {
        _active_shader = shader;
    #ifdef DEBUG
        glValidateProgram(shader->program);
        if(_check_shader_error(shader->program, GL_VALIDATE_STATUS, 1) == GFX_FAILURE){
            return ;
        }
    #endif

        glUseProgram(shader->program); //Attaching shaders    
    }
    
}

#define _GET_UNIFORM_LOC(out_loc, shader, name)                     \
    int out_loc = glGetUniformLocation(shader->program, name);      \
    if(out_loc == -1)                                               \
    {                                                               \
        nb_warn("Could not set uniform (%s)\n",name);               \
        return GFX_FAILURE;                                         \
    }                                                               \



gfx_status  gfx_set_uniform_float(gfx_shader * shader, const char * name, float value)
{
    _GET_UNIFORM_LOC(location, shader, name)
    glUniform1f(location, value); 
    return GFX_SUCCESS;
}
gfx_status  gfx_set_uniform_int(gfx_shader * shader, const char * name, int value)
{
    _GET_UNIFORM_LOC(location, shader, name)
    glUniform1i(location, value); 
    return GFX_SUCCESS;
}

gfx_status  gfx_set_uniform_vec2f(gfx_shader * shader, const char * name, const vec2f * value)
{
    _GET_UNIFORM_LOC(location, shader, name)
    glUniform2f(location, value->x,value->y); 
    return GFX_SUCCESS;
}
gfx_status  gfx_set_uniform_vec2i(gfx_shader * shader, const char * name, const vec2i * value)
{
    _GET_UNIFORM_LOC(location, shader, name)
    glUniform2i(location, value->x,value->y); 
    return GFX_SUCCESS;
}

gfx_status gfx_init_mesh(gfx_mesh * mesh, gfx_shader * shader, gfx_vertex *  verts, int num_verts)
{
    int stride = sizeof(gfx_vertex);
    int size = sizeof(gfx_vertex)*num_verts;        

    mesh->verts = verts;
    mesh->num_verts = num_verts;
    mesh->shader = shader;
    
    gfx_bind_shader(shader); //Attaching shaders

    glGenVertexArrays(1, &mesh->vao);
    glBindVertexArray(mesh->vao);

    glGenBuffers(1, &mesh->vbo);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, size, verts, GL_STATIC_DRAW);

    glVertexAttribPointer(shader->vert_loc, 2, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(shader->vert_loc);//enable to draw
    
    glVertexAttribPointer(shader->uv_loc, 2, GL_UNSIGNED_BYTE, GL_FALSE, stride, (void*)offsetof(gfx_vertex, uv));
    glEnableVertexAttribArray(shader->uv_loc);//enable to draw
    
    return GFX_SUCCESS;
}

void gfx_destroy_mesh(gfx_mesh * mesh)
{
    gfx_bind_shader(mesh->shader);
    glDeleteVertexArrays(1, &mesh->vao);
    glDeleteBuffers(1, &mesh->vbo);
    mesh->verts=0;
    
}
void gfx_render_mesh(gfx_mesh * mesh)
{
    glBindVertexArray(mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glDrawArrays(GL_TRIANGLES, 0,  mesh->num_verts);
    glBindVertexArray(0);

}

// ----------------------------------------------------- Texture ---------------------------------------------------
void gfx_bind_texture(u32 texture_location, gfx_texture * texture)
{
    u32 tex_unit = GL_TEXTURE0+texture_location;
    glActiveTexture(tex_unit);
    glBindTexture(texture->type, texture->handle);
}


gfx_status  gfx_init_texture(u32 texture_location, gfx_texture * texture, gfx_texture_type type, gfx_format data_format, byte  * data, int width, int height)
{ 
    texture->width = width;
    texture->height = height;
    texture->data = data;

    i32 format, internal_format; 
    texture->format = data_format;
    switch(data_format)
    {
        case GFX_RGBA8: 
            format          = GL_RGBA_INTEGER;  
            internal_format = GL_RGBA8UI;  
        break;
        case GFX_RGB8: 
            format          = GL_RGB_INTEGER;  
            internal_format = GL_RGB8UI;  
        break;
        case GFX_RG8: 
            format          = GL_RG_INTEGER;
            internal_format = GL_RG8UI;
        break;
        case GFX_R8: 
            format          = GL_RED_INTEGER;
            internal_format = GL_R8UI;
        break;
        default:
            nb_error("GFX: load texture : invalid in format  (%d)",data_format);
            return GFX_FAILURE;
        break;
    }

    switch(type)
    {
        case  GFX_TEXTURE_1D: 
            texture->type = GL_TEXTURE_1D;  
        break;
        case GFX_TEXTURE_2D: 
            texture->type = GL_TEXTURE_2D;  
        break;
        default:
            nb_error("GFX: load texture : invalid value (%d)",type);
            return GFX_FAILURE;
        break;
    }

    glGenTextures(1, &texture->handle);
    glBindTexture(texture->type, texture->handle);

    //gfx_bind_texture(texture_location, texture);
    
    glTexParameteri(texture->type, GL_TEXTURE_WRAP_S,   GL_REPEAT);
    glTexParameteri(texture->type, GL_TEXTURE_WRAP_T,   GL_REPEAT);
    glTexParameteri(texture->type, GL_TEXTURE_MIN_FILTER,   GL_NEAREST);
    glTexParameteri(texture->type, GL_TEXTURE_MAG_FILTER,   GL_NEAREST);


    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    switch(type)
    {
        case  GFX_TEXTURE_1D: 
            glTexImage1D(texture->type, 0, internal_format, width, 0, format, GL_UNSIGNED_BYTE, data);
        break;
        case GFX_TEXTURE_2D: 
            glTexImage2D(texture->type, 0, internal_format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        break;
        default:
            nb_error("GFX: load texture : invalid texture type(%d)",type);
            return GFX_FAILURE;
        break;
    }

    
    //float border_color[] = { 1.0, 1.0, 1.0 };
    //glTexParameterfv(texture->type, GL_TEXTURE_BORDER_COLOR, border_color);
 
    return GFX_SUCCESS;
}

void gfx_update_texture(u32 texture_location, gfx_texture * texture, int x, int y, int width, int height)
{

    gfx_bind_texture(texture_location, texture);
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage2D(texture->type, 0,x,y,width,height,texture->format, GL_UNSIGNED_BYTE, texture->data);
    //float border_color[] = { 1.0, 1.0, 1.0 };
    //glTexParameterfv(texture->type, GL_TEXTURE_BORDER_COLOR, border_color);

}

// ------------------------------------- Rect -------------------------------------------
gfx_status  gfx_init_rect(gfx_rect * rect, gfx_shader * shader, float x, float y, float w, float h)
{
 
    gfx_vertex * verts = nb_malloc(6*sizeof(gfx_vertex));
    const float xw = x+w;
    const float yh = y+h;
    rect->pos.x = x;       rect->pos.y =  y;
    rect->size.x= w;       rect->size.y=  h;
    verts[0].pos.x =0;     verts[0].pos.y =0;
    verts[1].pos.x =0;     verts[1].pos.y =1;
    verts[2].pos.x =1;     verts[2].pos.y =1;
    verts[3].pos.x =0;     verts[3].pos.y =0;
    verts[4].pos.x =1;     verts[4].pos.y =0;
    verts[5].pos.x =1;     verts[5].pos.y =1;

    verts[0].uv.x  =0;     verts[0].uv.y  =0;
    verts[1].uv.x  =0;     verts[1].uv.y  =1;
    verts[2].uv.x  =1;     verts[2].uv.y  =1;
    verts[3].uv.x  =0;     verts[3].uv.y  =0;
    verts[4].uv.x  =1;     verts[4].uv.y  =0;
    verts[5].uv.x  =1;     verts[5].uv.y  =1;
    
    rect->mesh.verts = verts;
    return gfx_init_mesh(&rect->mesh, shader, rect->mesh.verts, 6);
}


void  gfx_destroy_rect(gfx_rect * rect)
{
    nb_free(rect->mesh.verts);
    gfx_destroy_mesh(&rect->mesh);
}
void  gfx_render_rect(gfx_rect * rect)
{
    //Update uniforms only if dirty?
    gfx_set_uniform_vec2f(rect->mesh.shader, GFX_UNIFORM_SIZE, &rect->size);
    gfx_set_uniform_vec2f(rect->mesh.shader, GFX_UNIFORM_POS,  &rect->pos);
    gfx_render_mesh(&rect->mesh);   
}

// ------------ GFX Palette -----------------------

void        gfx_init_palette(gfx_palette *palette, rgb * data, u32 width, u32 height)
{

    gfx_status status = gfx_init_texture(    
                            GFX_PALETTE_TEXTURE_UNIT,
                            &palette->texture, 
                            GFX_TEXTURE_1D,
                            GFX_RGB8,
                            &data->data[0],
                            width, height);
    if(status == GFX_FAILURE)
    {
        nb_error("Failed to init palette texture");
        return;
    } 
    palette->data = data;

}
void        gfx_use_palette(gfx_palette *palette )
{
    gfx_bind_texture(GFX_PALETTE_TEXTURE_UNIT, &(palette->texture));
}

void        gfx_update_palette(gfx_palette *palette)
{
    gfx_update_texture( GFX_PALETTE_TEXTURE_UNIT, &palette->texture,0,0, palette->texture.width, palette->texture.height);
}
//if data changes

void        gfx_destroy_palette(gfx_palette * palette)
{
    palette->data=0;
}


// ------------ GFX atlas -----------------------

void        gfx_init_atlas(gfx_atlas * atlas, byte * indices, u32 width, u32 height)
{

    gfx_status status = gfx_init_texture(    
                            GFX_ATLAS_TEXTURE_UNIT,
                            &atlas->texture, 
                            GFX_TEXTURE_2D, 
                            GFX_R8,
                            &indices[0],
                            width, height);
    if(status == GFX_FAILURE)
    {
        nb_error("Failed to init palette texture");
        return;
    } 
    atlas->indices = indices;
}

void        gfx_destroy_atlas(gfx_atlas * atlas)
{
    atlas->indices =0 ;
//    nb_free(atlas->indices);
}

void        gfx_update_atlas(gfx_atlas *atlas)
{
    gfx_update_texture( GFX_ATLAS_TEXTURE_UNIT, &atlas->texture,0,0, atlas->texture.width, atlas->texture.height); 
}

void        gfx_use_atlas(gfx_atlas * atlas)
{
    gfx_bind_texture(GFX_ATLAS_TEXTURE_UNIT, &(atlas->texture));
}


// ------------ GFX Sheet -----------------------
void        gfx_use_sheet(gfx_sheet * sheet)
{
    
    gfx_use_palette(sheet->palette);
    gfx_use_atlas(sheet->atlas);
    
}

void        gfx_init_sheet(gfx_sheet * sheet, gfx_palette * palette, gfx_atlas * atlas)
{
    sheet->atlas = atlas;
    sheet->palette = palette;
}


//---------------------------------------------- Sprites --------------------------------------------------

gfx_status  gfx_init_sprite(gfx_sprite * sprite, gfx_sheet * sheet, gfx_shader * shader, vec2i offset, gfx_sprite_type type)
{
    int width, height;
    switch(type)
    {
        //Regular 8x8
        case GFX_SPRITE_REGULAR:
            width  = NB_SPRITE_SIZE;
            height = NB_SPRITE_SIZE;
        break;
        //Tall 8x16    
        case GFX_SPRITE_WIDE:
            width  = NB_SPRITE_SIZE*2;
            height = NB_SPRITE_SIZE;
        break;
         //Wide 16x8
        case GFX_SPRITE_TALL:
            width  = NB_SPRITE_SIZE;
            height = NB_SPRITE_SIZE*2;
        break;
    }

//TODO type to determine 
    gfx_status  status = gfx_init_rect(&sprite->rect, shader, 0, 0, width, height);
    if(status == GFX_FAILURE)
    {
        return GFX_FAILURE;
    }

    sprite->sheet = sheet;
    sprite->offset = offset;
    sprite->flip_x = sprite->flip_y = 0; 
}

void        gfx_destroy_sprite(gfx_sprite * sprite)
{
    gfx_destroy_rect(&sprite->rect);
}

void        gfx_render_sprite(gfx_sprite * sprite)
{
    gfx_use_sheet(sprite->sheet);
    //sprites reference sader
    gfx_set_uniform_vec2i(sprite->rect.mesh.shader, GFX_UNIFORM_OFFSET, &sprite->offset);
    gfx_render_rect(&sprite->rect);
}


void        gfx_get_sprite_xy(gfx_sprite * sprite, float * x, float * y )
{
    *x = sprite->rect.pos.x;
    *y = sprite->rect.pos.y;
}

void        gfx_set_sprite_xy(gfx_sprite * sprite, float  x, float  y )
{
    sprite->rect.pos.x = x;
    sprite->rect.pos.y = y;
}

void        gfx_flip_sprite(gfx_sprite * sprite, bool flip_x, bool flip_y )
{
    sprite->flip_x = flip_x;
    sprite->flip_y = flip_y;   
}
