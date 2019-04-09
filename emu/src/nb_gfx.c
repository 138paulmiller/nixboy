

#include "nb_gfx.h"
#include "nb_gc.h"

#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <SDL2/SDL.h>

//create window object
static SDL_Window*     _sdl_window;
static SDL_Event*      _sdl_event;
static SDL_GLContext   _sdl_nb_context;
static nb_timer        _fps_timer;  //create a shader stack to push and pop?

#define NB_KEY_COUNT 512

//Input objects
//native keycodes map to this index. Uses get_keycode to map 
static nb_key_mode     _keys[NB_KEY_COUNT ] = {NB_KEYUP };
static nb_mouse        _mouse;

//Frames per Sec, Frames Per MilliSec
static int  _fps_cap  = -1;
static int _fpms_cap  = -1;

static nb_shader *     _active_shader;  //create a shader stack to push and pop?
//map all indices from 1d to 2d using size of palette, sheet or map
// x = i/w
// y = i%w

//translates from internel keycode to exposed nb_key codes
static u32 get_keycode(nb_keycode keycode)
{
static SDL_Keycode keys_nb_to_sdl[NB_KEY_COUNT] = { 
    SDLK_UNKNOWN     ,                   
    SDLK_BACKSPACE   ,                    
    SDLK_TAB         ,            
    SDLK_RETURN      ,                  
    SDLK_ESCAPE      ,                
    SDLK_SPACE       ,             
    SDLK_EXCLAIM     ,                 
    SDLK_QUOTEDBL    ,               
    SDLK_HASH        ,                 
    SDLK_DOLLAR      ,                
    SDLK_PERCENT     ,                
    SDLK_AMPERSAND   ,                   
    SDLK_QUOTE       ,              
    SDLK_LEFTPAREN   ,                   
    SDLK_RIGHTPAREN  ,                      
    SDLK_ASTERISK    ,                   
    SDLK_PLUS        ,               
    SDLK_COMMA       ,                
    SDLK_MINUS       ,              
    SDLK_PERIOD      ,                  
    SDLK_SLASH       ,            
    SDLK_0           ,             
    SDLK_1           ,        
    SDLK_2           ,             
    SDLK_3           ,        
    SDLK_4           ,             
    SDLK_5           ,        
    SDLK_6           ,             
    SDLK_7           ,        
    SDLK_8           ,             
    SDLK_9           ,        
    SDLK_COLON       ,                 
    SDLK_SEMICOLON   ,                
    SDLK_LESS        ,                 
    SDLK_EQUALS      ,                
    SDLK_GREATER     ,                
    SDLK_QUESTION    ,                   
    SDLK_AT          ,          
    SDLK_LEFTBRACKET ,                  
    SDLK_BACKSLASH   ,                     
    SDLK_RIGHTBRACKET,                    
    SDLK_CARET       ,                 
    SDLK_UNDERSCORE  ,                   
    SDLK_BACKQUOTE   ,                   
    SDLK_a           ,          
    SDLK_b           ,           
    SDLK_c           ,          
    SDLK_d           ,           
    SDLK_e           ,          
    SDLK_f           ,           
    SDLK_g           ,          
    SDLK_h           ,           
    SDLK_i           ,          
    SDLK_j           ,           
    SDLK_k           ,          
    SDLK_l           ,           
    SDLK_m           ,          
    SDLK_n           ,           
    SDLK_o           ,          
    SDLK_p           ,           
    SDLK_q           ,          
    SDLK_r           ,           
    SDLK_s           ,          
    SDLK_t           ,           
    SDLK_u           ,          
    SDLK_v           ,           
    SDLK_w           ,          
    SDLK_x           ,           
    SDLK_y           ,          
    SDLK_z           ,           
    SDLK_DELETE
};
    return keys_nb_to_sdl[keycode];
}

static nb_status _check_shader_error(int shader, int  flag, int is_program)
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

static  nb_status _create_shader_stage(uint type, const char* source)
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

    _sdl_nb_context = SDL_GL_CreateContext(_sdl_window);
    int status = glewInit(); 
    if(status != GLEW_OK)
    {
        nb_error("GLEW Failed to initialize %d", status);
        return NB_FAILURE; 
    }   

    _active_shader = 0;
    nb_timer_init(&_fps_timer);
    return NB_SUCCESS;

}


void nb_destroy_window()
{
    SDL_DestroyWindow(_sdl_window);
    SDL_GL_DeleteContext(_sdl_nb_context);
    SDL_Quit();
}

void nb_clear_window()
{
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


nb_status nb_update_window()
{
    //Poll input events
    static SDL_Event event;
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
                _keys[event.key.keysym.sym] = NB_KEYUP;
            }
            break;
            case SDL_KEYDOWN:
            {
                u32 sym = event.key.keysym.sym;
                nb_key_mode key = _keys[sym];
                //if was previously down
                if (key == NB_KEYDOWN)
                {
                    _keys[sym]= NB_KEYHOLD;
                } 
                else
                {
                    _keys[sym]= NB_KEYDOWN;                    
                }
            }
            break;


        }
        
    }



    SDL_GL_SwapWindow(_sdl_window);
    nb_clear_window();
    nb_timer_tick(&_fps_timer); 
    if(_fps_cap != -1 && nb_fpms() < _fpms_cap)
    {
        SDL_Delay(( _fpms_cap - nb_fpms() ));
    }
    return NB_SUCCESS;
}

nb_key_mode nb_key(u32 keycode)
{

    return _keys[get_keycode( keycode)];

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

    shader->program = glCreateProgram();
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
        return NB_FAILURE;
    }
    if(shader->uv_loc == -1)
    {
        nb_error("GFX Invalid name for UV Attrib: Expected %s", NB_ATTRIB_UV);
        return NB_FAILURE;
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
        _active_shader = shader;
    #ifdef DEBUG
        glValidateProgram(shader->program);
        if(_check_shader_error(shader->program, GL_VALIDATE_STATUS, 1) == NB_FAILURE){
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
        /*nb_warn("Could not set uniform (%s)\n",name);*/               \
        return NB_FAILURE;                                         \
    }                                                               \



nb_status  nb_set_uniform_float(nb_shader * shader, const char * name, float value)
{
    if(!shader) return;
    _GET_UNIFORM_LOC(location, shader, name)
    glUniform1f(location, value); 
    return NB_SUCCESS;
}
nb_status  nb_set_uniform_int(nb_shader * shader, const char * name, int value)
{
    if(!shader) return;
    _GET_UNIFORM_LOC(location, shader, name)
    glUniform1i(location, value); 
    return NB_SUCCESS;
}

nb_status  nb_set_uniform_vec2f(nb_shader * shader, const char * name, const vec2f * value)
{
    if(!shader) return;
    _GET_UNIFORM_LOC(location, shader, name)
    glUniform2f(location, value->x,value->y); 
    return NB_SUCCESS;
}
nb_status  nb_set_uniform_vec2i(nb_shader * shader, const char * name, const vec2i * value)
{
    if(!shader) return;
    _GET_UNIFORM_LOC(location, shader, name)
    glUniform2i(location, value->x,value->y); 
    return NB_SUCCESS;
}

nb_status nb_init_mesh(nb_mesh * mesh, nb_shader * shader, nb_vertex *  verts, int num_verts)
{
    if(!shader || !mesh ) return;
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
    u32 tex_unit = GL_TEXTURE0+texture_location;
    glActiveTexture(tex_unit);
    glBindTexture(texture->type, texture->handle);
}


nb_status  nb_init_texture(u32 texture_location, nb_texture * texture, nb_texture_type type, nb_format data_format, byte  * data, int width, int height)
{ 
    texture->width = width;
    texture->height = height;
    texture->data = data;

    i32 format, internal_format; 
    texture->format = data_format;
    switch(data_format)
    {
        case NB_RGBA8: 
            format          = GL_RGBA_INTEGER;  
            internal_format = GL_RGBA8UI;  
        break;
        case NB_RGB8: 
            format          = GL_RGB_INTEGER;  
            internal_format = GL_RGB8UI;  
        break;
        case NB_RG8: 
            format          = GL_RG_INTEGER;
            internal_format = GL_RG8UI;
        break;
        case NB_R8: 
            format          = GL_RED_INTEGER;
            internal_format = GL_R8UI;
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
    glBindTexture(texture->type, texture->handle);

    //nb_bind_texture(texture_location, texture);
    
    glTexParameteri(texture->type, GL_TEXTURE_WRAP_S,   GL_REPEAT);
    glTexParameteri(texture->type, GL_TEXTURE_WRAP_T,   GL_REPEAT);
    glTexParameteri(texture->type, GL_TEXTURE_MIN_FILTER,   GL_NEAREST);
    glTexParameteri(texture->type, GL_TEXTURE_MAG_FILTER,   GL_NEAREST);


    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    switch(type)
    {
        case  NB_TEXTURE_1D: 
            glTexImage1D(texture->type, 0, internal_format, width, 0, format, GL_UNSIGNED_BYTE, data);
        break;
        case NB_TEXTURE_2D: 
            glTexImage2D(texture->type, 0, internal_format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
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
    glTexSubImage2D(texture->type, 0,x,y,width,height,texture->format, GL_UNSIGNED_BYTE, texture->data);
    //float border_color[] = { 1.0, 1.0, 1.0 };
    //glTexParameterfv(texture->type, GL_TEXTURE_BORDER_COLOR, border_color);

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
    nb_set_uniform_vec2f(rect->mesh.shader, NB_UNIFORM_SIZE, &rect->size);
    nb_set_uniform_vec2f(rect->mesh.shader, NB_UNIFORM_POS,  &rect->pos);
    nb_render_mesh(&rect->mesh);   
}

// ------------ GFX Palette -----------------------

void        nb_init_palette(nb_palette *palette, rgb * data, u32 width, u32 height)
{

    nb_status status = nb_init_texture(    
                            NB_TEXTURE_UNIT_PALETTE,
                            &palette->texture, 
                            NB_TEXTURE_1D,
                            NB_RGB8,
                            &data->data[0],
                            width, height);
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
    nb_update_texture( NB_TEXTURE_UNIT_PALETTE, &palette->texture,0,0, palette->texture.width, palette->texture.height);
}
//if data changes

void        nb_destroy_palette(nb_palette * palette)
{
    if(! palette) return;

    palette->data=0;
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
    if(atlas)
        atlas->indices =0 ;
//    nb_free(atlas->indices);
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
        nb_set_uniform_vec2i(sprite->rect.mesh.shader, NB_UNIFORM_OFFSET, &sprite->offset);
        nb_render_rect(&sprite->rect);
    }
}


void        nb_get_sprite_xy(nb_sprite * sprite, float * x, float * y )
{
    *x = sprite->rect.pos.x;
    *y = sprite->rect.pos.y;
}

void        nb_set_sprite_xy(nb_sprite * sprite, float  x, float  y )
{
    sprite->rect.pos.x = x;
    sprite->rect.pos.y = y;
}

void        nb_flip_sprite(nb_sprite * sprite, bool flip_x, bool flip_y )
{
    sprite->flip_x = flip_x;
    sprite->flip_y = flip_y;   
}
