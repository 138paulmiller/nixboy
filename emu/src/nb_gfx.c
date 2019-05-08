

#include "nb_gfx.h"
#include "nb_gc.h"

#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>

#include <SDL2/SDL.h>

//create window object
static SDL_Window*     _sdl_window;
static SDL_Event*      _sdl_event;
static SDL_GLContext   _sdl_gl_context;


static nb_mouse        _mouse;
static nb_timer        _fps_timer;  //create a shader stack to push and pop?



bool _suppress_uniform_error  = 0;
//Frames per Sec, Frames Per MilliSec
static int  _fps_cap  = -1;
static int _fpms_cap  = -1;

static nb_shader *     _active_shader;  //create a shader stack to push and pop?
//map all indices from 1d to 2d using size of palette, sheet or map
// x = i/w
// y = i%w
static nb_status _check_shader_error(i32 shader, i32  flag, bool is_program)
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
        return NB_FAILURE;
    }
    
    return NB_SUCCESS;
}

static  nb_status _create_shader_stage(u32 type, const char* source)
{

    int shader = glCreateShader(type);
    const int count = 1;
    const char * files[] = { source };
    int lengths[]       = { strlen(source)  };
    glShaderSource(shader, count, files, lengths);
    glCompileShader(shader);
    
    if(_check_shader_error(shader, GL_COMPILE_STATUS,0) == NB_FAILURE){
        return -1;
    }
    return shader;

}


/////////////////////////////////////// Window api ///////////////////////////////////////////////////


 nb_status nb_init_window(const char * title, int width, int height)
{
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0){
        puts("SDL Failed to initialize!"); exit(0);     
    }        
    
    _sdl_window = SDL_CreateWindow(title,SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    width, height, SDL_WINDOW_OPENGL);
    //Initialize opengl nb_color attributes buffer size
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 2);

    _sdl_gl_context = SDL_GL_CreateContext(_sdl_window);
    if(_sdl_gl_context == 0)
    {
        nb_error("SDL Failed to create GL Context ");
        return NB_FAILURE;     
    }

    nb_debug( "GL Context "  ) ;
    nb_debug( "Vendor  : %s\n"  , glGetString( GL_VENDOR ) ) ;
    nb_debug( "Renderer: %s\n" , glGetString( GL_RENDERER ) ) ;
    nb_debug( "Version : %s\n" , glGetString( GL_VERSION ) ) ;

    int status = glewInit(); 
    if(status != GLEW_OK)
    {
        nb_error("GLEW Failed to initialize ");
        return NB_FAILURE; 
    }   

    _active_shader = 0;
    nb_timer_init(&_fps_timer);
    return NB_SUCCESS;

}


void nb_destroy_window()
{

    SDL_DestroyWindow(_sdl_window);
    SDL_GL_DeleteContext(_sdl_gl_context);
    SDL_Quit();

}

void nb_clear_window()
{
    glColorMask(1,1,1,1);
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}



nb_status nb_update_window()
{
    static u32 key_stack[NB_KEY_COUNT] = {0};
    static i16 key_stack_index         = -1;
    
    //if a key was previously down. push, then next frame set to hold
    while(key_stack_index > -1)
    {
        u32 prev_key = key_stack[key_stack_index];
        switch(nb_get_native_keystate(prev_key))
        {
            case  NB_KEYPRESS:   
            {
                nb_set_native_keystate(prev_key, NB_KEYDOWN);      
            }
            break;
        }

        key_stack_index--;
    }

    static SDL_Event event;
    //handle input events
    while (SDL_PollEvent(&event))
    {
        switch(event.type)
        {
            case  SDL_QUIT:  return NB_FAILURE;
            
            case  SDL_MOUSEMOTION: 
            {
                _mouse.x = event.motion.x;
                _mouse.y = event.motion.y;
            }
            break;

            case  SDL_KEYUP: 
            {
                nb_set_native_keystate(event.key.keysym.sym, NB_KEYUP);
            }
            break;
            case SDL_KEYDOWN:
            {
                u32 key = key_stack[++key_stack_index] = event.key.keysym.sym; 
                if(nb_get_native_keystate(key) != NB_KEYDOWN)
                {
                    nb_set_native_keystate(key, NB_KEYPRESS);      
                }
            }
            break;
        }
    }
    
    // enable transprency 
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    SDL_GL_SwapWindow(_sdl_window);
    nb_clear_window();


    nb_timer_tick(&_fps_timer); 
    if(_fps_cap != -1 && nb_fpms() < _fpms_cap)
    {
//        SDL_Delay(( _fpms_cap - nb_fpms() ));
    }
    return NB_SUCCESS;
}

void nb_get_mouse_xy(u16 *x, u16 * y)
{
    *x = _mouse.x;
    *y = _mouse.y;
}

/////------------------------------ End Window

//--------------------------------- FPS Utiltites

void nb_cap_fps(int max_fps)
{
    _fps_cap  =max_fps;
    _fpms_cap  = 1000/_fps_cap;
}

void nb_uncap_fps()
{
     _fps_cap  =_fpms_cap = -1;
}

// ---------------- FPS Utils-------------------------------------
float  nb_delta_sec()
{
    return 0.001 * nb_delta_ms();
}
float  nb_delta_ms()
{
    return (_fps_timer.delta_ticks * 1000.f) /(float)SDL_GetPerformanceFrequency() ;
}
float  nb_fps()
{
    return 1.0f/ (nb_delta_sec());
}

float  nb_fpms()
{
    return 1.0f/ (nb_delta_ms());
}

void  nb_timer_init(nb_timer * timer){
    timer->delta_ticks = 0;
    timer->now_ticks =0;    
    timer->last_ticks = 0;
}

void nb_timer_tick(nb_timer * timer)
{

    timer->last_ticks = timer->now_ticks;
    timer->now_ticks = SDL_GetPerformanceCounter();
    timer->delta_ticks = (timer->now_ticks - timer->last_ticks);
}   //elapsed time while paused or not



// -------------------------- Shader ---------------------------------------
 nb_status  nb_init_shader(nb_shader * shader, const char * vertex_source, const char * fragment_source)
{

    u32 program = glCreateProgram();
    if(program == 0)
    {
        nb_error("Failed to Create Shader Program");
        return NB_FAILURE;
    }
    
    shader->program = program;
    shader->vert_shader   = _create_shader_stage(GL_VERTEX_SHADER, vertex_source);

    if(shader->vert_shader == -1)
    {
        nb_error("Failed to Init Vertex Shader");
        return NB_FAILURE;
    }
    shader->frag_shader   = _create_shader_stage(GL_FRAGMENT_SHADER,fragment_source);
    if(shader->frag_shader == -1)
    {
        nb_error("Failed to Init Fragment Shader");
        return NB_FAILURE;
    }
    
    glAttachShader(shader->program,  shader->vert_shader);
    glAttachShader(shader->program,  shader->frag_shader);
    glLinkProgram(shader->program);

    if(_check_shader_error(shader->program, GL_LINK_STATUS, 1)== NB_FAILURE){
        return NB_FAILURE;
    }

    
    shader->vert_loc = glGetAttribLocation(shader->program, NB_ATTRIB_VERT);
    shader->uv_loc = glGetAttribLocation(shader->program, NB_ATTRIB_UV);
    if(shader->vert_loc == -1)
    {
        nb_error("GFX Invalid name for Position Attrib: Expected %s", NB_ATTRIB_VERT);
        //return NB_FAILURE;
    }
    if(shader->uv_loc == -1)
    {
        nb_error("GFX Invalid name for UV Attrib: Expected %s", NB_ATTRIB_UV);
       // return NB_FAILURE;
    }


    return NB_SUCCESS;

}
void    nb_destroy_shader(nb_shader * shader)
{
    if(!shader) return;

    glDeleteShader(shader->vert_shader); // removes
    glDeleteShader(shader->frag_shader); // removes
    glDeleteProgram(shader->program);

}

//bind only if not already bound
void    nb_bind_shader(nb_shader * shader)
{
    if(!shader) return;

    if(_active_shader != shader)
    {
    #ifdef DEBUG
        glValidateProgram(shader->program);
        if(_check_shader_error(shader->program, GL_VALIDATE_STATUS, 1) == NB_FAILURE)
        {
            nb_error("Trying to bind invalid shader!\n")
            return ;
        }
    #endif

        _active_shader = shader;
        glUseProgram(shader->program); //Attaching shaders    
    }
    
}

#define _GET_UNIFORM_LOC(out_loc, shader, name)                                 \
    int out_loc = glGetUniformLocation(shader->program, name);                  \
    if(out_loc == -1){                                                          \
        if(_suppress_uniform_error){nb_error("Could not set uniform (%s)\n",name); exit(0); }  \
        return NB_FAILURE;}                                                               



nb_status  nb_set_uniform_float(nb_shader * shader, const char * name, float value)
{
    if(!shader) return NB_FAILURE;
    _GET_UNIFORM_LOC(location, shader, name)
    glUniform1f(location, value); 
    return NB_SUCCESS;
}
nb_status  nb_set_uniform_int(nb_shader * shader, const char * name, int value)
{
    if(!shader) return NB_FAILURE;
    _GET_UNIFORM_LOC(location, shader, name)
    glUniform1i(location, value); 
    return NB_SUCCESS;
}

nb_status  nb_set_uniform_vec2f(nb_shader * shader, const char * name, const vec2f * value)
{
    if(!shader) return NB_FAILURE;
    _GET_UNIFORM_LOC(location, shader, name)
    glUniform2f(location, value->x,value->y); 
    return NB_SUCCESS;
}
nb_status  nb_set_uniform_vec2i(nb_shader * shader, const char * name, const vec2i * value)
{
    if(!shader) return NB_FAILURE;
    _GET_UNIFORM_LOC(location, shader, name)
    glUniform2i(location, value->x,value->y); 
    return NB_SUCCESS;
}

#undef _GET_UNIFORM_LOC
#undef _suppress_uniform_error

nb_status nb_init_mesh(nb_mesh * mesh, nb_shader * shader, nb_vertex *  verts, int num_verts)
{
    if(!shader || !mesh ) return NB_FAILURE;
    int stride = sizeof(nb_vertex);
    int size = sizeof(nb_vertex)*num_verts;        

    mesh->verts = verts;
    mesh->num_verts = num_verts;
    mesh->shader = shader;
    
    nb_bind_shader(shader); //Attaching shaders

    glGenVertexArrays(1, &mesh->vao);
    glBindVertexArray(mesh->vao);

    glGenBuffers(1, &mesh->vbo);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, size, verts, GL_STATIC_DRAW);

    glVertexAttribPointer(shader->vert_loc, 2, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(shader->vert_loc);//enable to draw
    
    glVertexAttribPointer(shader->uv_loc, 2, GL_UNSIGNED_BYTE, GL_FALSE, stride, (void*)offsetof(nb_vertex, uv));
    glEnableVertexAttribArray(shader->uv_loc);//enable to draw
    
    return NB_SUCCESS;
}

void nb_destroy_mesh(nb_mesh * mesh)
{
    if(!mesh) return;
    nb_bind_shader(mesh->shader);
    glDeleteVertexArrays(1, &mesh->vao);
    glDeleteBuffers(1, &mesh->vbo);
    mesh->verts=0;
    
}
void nb_render_mesh(nb_mesh * mesh)
{


    glBindVertexArray(mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glDrawArrays(GL_TRIANGLES, 0,  mesh->num_verts);
    glBindVertexArray(0);

}

// ----------------------------------------------------- Texture ---------------------------------------------------
void nb_bind_texture(u32 texture_location, nb_texture * texture)
{
    //set uniform value to location of    
    u32 tex_unit = GL_TEXTURE0+texture_location;

    glActiveTexture(tex_unit);
    glBindTexture(texture->type, texture->handle);
}


nb_status  nb_init_texture( u32 texture_location, 
                            nb_texture * texture, 
                            nb_texture_type type, 
                            nb_format data_format, 
                            byte  * data, 
                            int width, 
                            int height)
{ 
    texture->width = width;
    texture->height = height;
    texture->data = data;
 
    switch(data_format)
    {
        case NB_RGBA8: 
            texture->format          = GL_RGBA_INTEGER;  
            texture->internal_format = GL_RGBA8UI;  
        break;
        case NB_RGB8: 
            texture->format          = GL_RGB_INTEGER;  
            texture->internal_format = GL_RGB8UI;  
        break;
        case NB_RG8: 
            texture->format          = GL_RG_INTEGER;
            texture->internal_format = GL_RG8UI;
        break;
        case NB_R8: 
            texture->format          = GL_RED_INTEGER;
            texture->internal_format = GL_R8UI;
        break;
        default:
            nb_error("GFX: load texture : invalid in format  (%d)",data_format);
            return NB_FAILURE;
        break;
    }

    switch(type)
    {
        case  NB_TEXTURE_1D: 
            texture->type = GL_TEXTURE_1D;  
        break;
        case NB_TEXTURE_2D: 
            texture->type = GL_TEXTURE_2D;  
        break;
        default:
            nb_error("GFX: load texture : invalid value (%d)",type);
            return NB_FAILURE;
        break;
    }

    glGenTextures(1, &texture->handle);
    nb_bind_texture(texture_location, texture);
        
    glTexParameteri(texture->type, GL_TEXTURE_WRAP_S,   GL_REPEAT);
    glTexParameteri(texture->type, GL_TEXTURE_WRAP_T,   GL_REPEAT);
    glTexParameteri(texture->type, GL_TEXTURE_MIN_FILTER,   GL_NEAREST);
    glTexParameteri(texture->type, GL_TEXTURE_MAG_FILTER,   GL_NEAREST);

    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    switch(type)
    {

        case  NB_TEXTURE_1D: 
            glTexImage1D(
                texture->type, //
                0,                      //level
                texture->internal_format,            
                width,                 
                0,                      //border
                texture->format,                 //
                GL_UNSIGNED_BYTE,       //data format 
                data                    
            );
        break;
        case NB_TEXTURE_2D: 

            glTexImage2D(
                texture->type, //
                0,                      //level
                texture->internal_format,            
                width, height,                 
                0,                      //border
                texture->format,                 //
                GL_UNSIGNED_BYTE,       //data format 
                data                    
            );
        break;
        default:
            nb_error("GFX: load texture : invalid texture type(%d)",type);
            return NB_FAILURE;
        break;
    }

    
    //float border_color[] = { 1.0, 1.0, 1.0 };
    //glTexParameterfv(texture->type, GL_TEXTURE_BORDER_COLOR, border_color);
 
    return NB_SUCCESS;
}

void nb_update_texture(u32 texture_location, nb_texture * texture, int x, int y, int width, int height)
{

    nb_bind_texture(texture_location, texture);
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1);

    switch(texture->type)
    {
        case  GL_TEXTURE_1D: 
            glTexSubImage1D(texture->type, 0,x,width,texture->format, GL_UNSIGNED_BYTE, texture->data);

        break;
        case GL_TEXTURE_2D: 
            glTexSubImage2D(texture->type, 0,x,y,width,height,texture->format, GL_UNSIGNED_BYTE, texture->data);
       break;
        default:
            nb_error("GFX: load texture : invalid texture type(%d)",texture->type);
            return;
        break;
    }


    

}
void nb_destroy_texture(nb_texture * texture)
{

    glDeleteTextures(1, &texture->handle);
}
// ------------------------------------- Rect -------------------------------------------
nb_status  nb_init_rect(nb_rect * rect, nb_shader * shader, float x, float y, float w, float h)
{
 
    nb_vertex * verts = nb_malloc(6*sizeof(nb_vertex));
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
    return nb_init_mesh(&rect->mesh, shader, rect->mesh.verts, 6);
}


void  nb_destroy_rect(nb_rect * rect)
{
    if(!rect) return;
    nb_free(rect->mesh.verts);
    nb_destroy_mesh(&rect->mesh);
}
void  nb_render_rect(nb_rect * rect)
{
    //Update uniforms only if dirty?
    nb_set_uniform_vec2f(rect->mesh.shader, NB_UNIFORM_RECT_SIZE, &rect->size);
    nb_set_uniform_vec2f(rect->mesh.shader, NB_UNIFORM_RECT_POS,  &rect->pos);
    nb_render_mesh(&rect->mesh);   
}

// ------------ GFX Palette -----------------------

void        nb_init_palette(nb_palette *palette, rgb * data, u32 width)
{

    nb_status status = nb_init_texture(    
                            NB_TEXTURE_UNIT_PALETTE,
                            &palette->texture, 
                            NB_TEXTURE_1D,
                            NB_RGBA8,
                            (byte*)data,
                            width, 1);
    if(status == NB_FAILURE)
    {
        nb_error("Failed to init palette texture");
        return;
    } 
    palette->data = data;

}
void        nb_use_palette(nb_palette *palette )
{
    nb_bind_texture(NB_TEXTURE_UNIT_PALETTE, &(palette->texture));
}

void        nb_update_palette(nb_palette *palette)
{
    nb_update_texture( NB_TEXTURE_UNIT_PALETTE,
         &palette->texture,0,0, palette->texture.width, palette->texture.height);
}
//if data changes

void        nb_destroy_palette(nb_palette * palette)
{
    if(! palette) return;
    palette->data=0;
    nb_destroy_texture(&palette->texture);
}


// ------------ GFX atlas -----------------------

void        nb_init_atlas(nb_atlas * atlas, byte * indices, u32 width, u32 height)
{

    nb_status status = nb_init_texture(    
                            NB_TEXTURE_UNIT_ATLAS,
                            &atlas->texture, 
                            NB_TEXTURE_2D, 
                            NB_R8,
                            &indices[0],
                            width, height);
    if(status == NB_FAILURE)
    {
        nb_error("Failed to init palette texture");
        return;
    } 
    atlas->indices = indices;
}

void        nb_destroy_atlas(nb_atlas * atlas)
{
    //destroy texture
    if(!atlas) return;

     atlas->indices =0 ;
    nb_destroy_texture(&atlas->texture);
}

void        nb_update_atlas(nb_atlas *atlas)
{
    nb_update_texture( NB_TEXTURE_UNIT_ATLAS, &atlas->texture,0,0, atlas->texture.width, atlas->texture.height); 
}

void        nb_use_atlas(nb_atlas * atlas)
{
    nb_bind_texture(NB_TEXTURE_UNIT_ATLAS, &(atlas->texture));
}



//---------------------------------------------- Sprites --------------------------------------------------

nb_status  nb_init_sprite(nb_sprite * sprite, nb_shader * shader, vec2i offset, vec2f size)
{
 
//TODO type to determine                                            //width , height
    nb_status  status = nb_init_rect(&sprite->rect, shader, 0, 0, size.x, size.y);
    if(status == NB_FAILURE)
    {
        nb_error("Failed to create sprte object");
        return NB_FAILURE;
    }
    sprite->is_active  = 1;
    sprite->offset = offset;
    sprite->flip_x = sprite->flip_y = 0; 
}

void        nb_destroy_sprite(nb_sprite * sprite)
{
    if(sprite)
    {
        sprite->is_active = 0;
        nb_destroy_rect(&sprite->rect);
    }   
}

void        nb_render_sprite(nb_sprite * sprite)
{
    if( sprite && sprite->is_active)
    {
        //sprites reference sader
        nb_set_uniform_vec2i(sprite->rect.mesh.shader, NB_UNIFORM_ATLAS_OFFSET, &sprite->offset);
        nb_render_rect(&sprite->rect);
    }
}

void        nb_move_sprite ( nb_sprite * sprite,
                                float  dx,   
                                float  dy    ) 
{

    sprite->rect.pos.x += dx;
    sprite->rect.pos.y += dy;

}

void        nb_get_sprite_xy(nb_sprite * sprite, float * x, float * y )
{
    *x = sprite->rect.pos.x;
    *y = sprite->rect.pos.y;
}

void        nb_set_sprite_xy(nb_sprite * sprite, float x, float y )
{
    sprite->rect.pos.x = x;
    sprite->rect.pos.y = y;
}

void        nb_set_sprite_offset(nb_sprite * sprite, float  offsetx, float offsety )
{
    sprite->offset.x = offsetx;
    sprite->offset.y = offsety;
}

void        nb_flip_sprite(nb_sprite * sprite, bool flip_x, bool flip_y )
{
    sprite->flip_x = flip_x;
    sprite->flip_y = flip_y;   
}

nb_status       nb_init_level(  nb_level * level,  
                                nb_shader * shader, 
                                byte * indices, 
                                u32 tile_width, u32 tile_height,
                                u32 level_width, u32 level_height
                                )
{
    
    nb_status status = nb_init_texture(    
                            NB_TEXTURE_UNIT_LEVEL,
                            &level->texture, 
                            NB_TEXTURE_2D, 
                            NB_R8,
                            &indices[0],
                            level_width, level_height);
    if(status == NB_FAILURE)
    {
        nb_error("Failed to init palette texture");
        return NB_FAILURE;
    } 
    status = nb_init_rect(
        &level->rect, 
        shader, 
        0, 0, 
        tile_width * level_width  , 
        tile_height * level_height
    );
    if(status == NB_FAILURE)
    {
        nb_error("Failed to create sprite object");
        return NB_FAILURE;
    } 
    level->scroll.x=0; 
    level->scroll.y=0;
    level->tile_size.x = tile_width  ;
    level->tile_size.y = tile_height ;
    level->indices = indices;
    return NB_SUCCESS;
}

void        nb_destroy_level  (nb_level * level )
{
    if(!level) return;

     level->indices =0 ;
    nb_destroy_texture(&level->texture);
    nb_destroy_rect(&level->rect);
}

void        nb_update_level   (nb_level * level)
{
    nb_update_texture( NB_TEXTURE_UNIT_LEVEL, &level->texture,0,0, level->texture.width, level->texture.height); 
}

void        nb_render_level      (nb_level * level)
{
    nb_set_uniform_vec2i(level->rect.mesh.shader, NB_UNIFORM_SCROLL,      &level->scroll);
    nb_set_uniform_vec2i(level->rect.mesh.shader, NB_UNIFORM_TILE_SIZE,   &level->tile_size);
    //sprites reference sader
    nb_bind_texture(NB_TEXTURE_UNIT_LEVEL, &(level->texture));

    nb_render_rect(&level->rect);
}

void        nb_scroll_level   (nb_level * level, int dx, int dy )
{
    level->scroll.x += dx; 
    level->scroll.y += dy;
    //clamp scroll?? Will wrap in shader

}
