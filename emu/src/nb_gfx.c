#include "nb_gfx.h"

//
#include <GL/glew.h>
#include <SDL2/SDL.h>

//Global State
static SDL_Window*     _sdl_window;
static SDL_Event*      _sdl_event;
static SDL_GLContext   _sdl_gfx_context;
static int _vert_shader, _frag_shader, _program;


//map all indices from 1d to 2d using size of palette, sheet or map
// x = i/w
// y = i%w

static int check_shader_error(int  shader, int  flag, int is_program)
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
        printf("Shader Log: %d %s \n", error_len, error_msg);
        nb_free( error_msg);
        return 0;
    }
    return 1;
}

static int create_shader_stage(uint type, const char* source)
{
    int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, 0);
    glCompileShader(shader);
    check_shader_error(shader, GL_COMPILE_STATUS,0);
    return shader;

}



void destroy_shader()
{
    glDeleteShader(_vert_shader); // removes
    glDeleteShader(_frag_shader); // removes
    glDeleteProgram(_program);

}


/////////////////////////////////////// Begin API ///////////////////////////////////////////////////


void gfx_init(const char * title, int width, int height)
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
        error("GLEW Failed to initialize %d", status);
        
    }   
}


void gfx_destroy()
{
    destroy_shader(); 

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
  //swap render and display buffers
    SDL_GL_SwapWindow(_sdl_window);
    //Poll input events
    SDL_Event event;
    while (SDL_PollEvent(&event))
        if (event.type == SDL_QUIT) return 0;
    return 1;

    gfx_clear();
    glUseProgram(_program); //Attaching shaders
}


void gfx_load_shader(const char * vertex_source, const char * fragment_source)
{
    _program = glCreateProgram();
    puts("Vert...");
    glAttachShader(_program, create_shader_stage(GL_VERTEX_SHADER, vertex_source) );
    puts("Frag....");
    glAttachShader(_program, create_shader_stage(GL_FRAGMENT_SHADER,fragment_source) );
    glLinkProgram(_program);
    if(check_shader_error(_program, GL_LINK_STATUS, 1))
        return;
    glValidateProgram(_program);
    if(check_shader_error(_program, GL_VALIDATE_STATUS, 1))return;
}


void gfx_load_mesh(gfx_mesh * mesh, gfx_vertex *  verts, int num_verts)
{
    int comp_size = sizeof(gfx_vertex);
    int stride = 4*sizeof(float);
    int size = sizeof(gfx_vertex)*num_verts;        

    mesh->verts = verts;
    mesh->num_verts = num_verts;
    glUseProgram(_program); //Attaching shaders

    int pos_loc = glGetAttribLocation(_program, GFX_ATTRIB_POS);
    int uv_loc = glGetAttribLocation(_program, GFX_ATTRIB_UV);
    if(pos_loc == -1)
        error("GFX Invalid name for Position Attrib: Expected %s", GFX_ATTRIB_POS);
    if(uv_loc == -1)
        error("GFX Invalid name for UV Attrib: Expected %s", GFX_ATTRIB_UV);
    glGenVertexArrays(1, &mesh->vao);
    glBindVertexArray(mesh->vao);

    glGenBuffers(1, &mesh->vbo);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, size, verts, GL_STATIC_DRAW);

    glVertexAttribPointer(pos_loc, 2, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(pos_loc);//enable to draw
    
    glVertexAttribPointer(uv_loc, 2, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(gfx_vertex, uv));
    glEnableVertexAttribArray(uv_loc);//enable to draw

}

void gfx_destroy_mesh(gfx_mesh * mesh)
{
    glUseProgram(_program); //Attaching shaders
    glDeleteVertexArrays(1, &mesh->vao);
    glDeleteBuffers(1, &mesh->vbo);
    mesh->verts=0;

}
void gfx_render(gfx_mesh * mesh)
{
    glBindVertexArray(mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glDrawArrays(GL_TRIANGLES, 0,  mesh->num_verts);
}


void gfx_bind_texture(gfx_texture * texture)
{
    glBindTexture(texture->type, texture->handle);
}


void  gfx_load_texture(gfx_texture * texture, gfx_texture_type type, gfx_format format, byte  * data, int width, int height)
{ 
    texture->width = width;
    texture->height = height;
    texture->data = data;

    switch(type)
    {
        case  GFX_TEXTURE_1D: 
            texture->type = GL_TEXTURE_1D;  
        break;
        case GFX_TEXTURE_2D: 
            texture->type = GL_TEXTURE_2D;  
        break;
        default:
            error("GFX: load texture : invalid format value (%d)",format);
        break;
    }
    switch(format)
    {
        case GFX_RGBA8: 
            texture->format = GL_RGBA;  
        break;
        case GFX_RGB8: 
            texture->format = GL_RGB;  
        break;
        case GFX_RG8: 
            texture->format = GL_RG;
        break;
        case GFX_R8: 
            texture->format = GL_RED;
        break;
        default:
            error("GFX: load texture : invalid format type (%d)",type);
        break;
    }
    glUseProgram(_program); 
    glGenTextures(1, &texture->handle);
    glBindTexture(texture->type, texture->handle);
    glTexParameteri(texture->type, GL_TEXTURE_WRAP_S,   GL_CLAMP_TO_BORDER);
    glTexParameteri(texture->type, GL_TEXTURE_WRAP_T,   GL_CLAMP_TO_BORDER);
    glTexParameteri(texture->type, GL_TEXTURE_MIN_FILTER,   GL_NEAREST);
    glTexParameteri(texture->type, GL_TEXTURE_MAG_FILTER,   GL_NEAREST);
    
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glTexImage2D(texture->type, 0, GL_RGBA8, width, height, 0, texture->format, GL_UNSIGNED_BYTE, data);

    float border_color[] = { 1.0, 1.0, 1.0 };
    glTexParameterfv(texture->type, GL_TEXTURE_BORDER_COLOR, border_color);
 
    glBindTexture(texture->type, 0); //unbind
   
}

void gfx_update_texture(gfx_texture * texture, int x, int y, int width, int height)
{
    glBindTexture(texture->type, texture->handle);
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage2D(texture->type, 0,x,y,width,height,texture->format, GL_UNSIGNED_BYTE, texture->data);
    float border_color[] = { 1.0, 1.0, 1.0 };
    glTexParameterfv(texture->type, GL_TEXTURE_BORDER_COLOR, border_color);
    glBindTexture(texture->type, 0);
}

void gfx_set_uniform(const char * name, float value)
{
    int location = glGetUniformLocation(_program, name);
    if(location == -1) 
            error("Could not set uniform (%s)",name);
    glUniform1f(location, value); 
}


void  gfx_init_rect(gfx_rect * rect, int x, int y, int w, int h)
{
    rect->mesh.verts = nb_malloc(6*sizeof(gfx_vertex));
    
        //pos,  uv
    //rect->mesh.verts[0] =  { {0,                   0                   },  {0,0}};
    //rect->mesh.verts[1] =  { {0,                   NB_SCREEN_HEIGHT/2  },  {0,1}};
    //rect->mesh.verts[2] =  { {NB_SCREEN_WIDTH/2,   NB_SCREEN_HEIGHT/2  },  {1,1}};        
    //rect->mesh.verts[3] =  { {0,                   0                   },  {0,0}};
    //rect->mesh.verts[4] =  { {NB_SCREEN_WIDTH/2,   0                   },  {1,0}};
    //rect->mesh.verts[5] =  { {NB_SCREEN_WIDTH/2,   NB_SCREEN_HEIGHT/2  },  {1,1}};    
    
    gfx_load_mesh(&rect->mesh, rect->mesh.verts, 6);
}


void  gfx_destroy_rect(gfx_rect * rect)
{
    free(rect->mesh.verts);
    gfx_destroy_mesh(&rect->mesh);
}