#include "nb_gfx.h"

//
#include <GL/glew.h>
#include <SDL2/SDL.h>

static SDL_Window*     _sdl_window;
static SDL_Event*      _sdl_event;
static SDL_GLContext   _sdl_gfx_context;

static int _vert_shader, _frag_shader, _program;


#define error(...) {printf(__VA_ARGS__);exit(-1);}


//map all indices from 1d to 2d using size of palette, sheet or map
// x = i/w
// y = i%w

static int check_shader_error(int  shader, int  flag, int isProgram)//////////////////remove eventually
{
    int status = 0;
    isProgram ?
        glGetProgramiv(shader, flag, &status)
        : glGetShaderiv(shader, flag, &status);
    if (status == GL_FALSE)
    {
        int errorLen = 0;
        isProgram ?
            glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &errorLen)
            : glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &errorLen);
        char* errorMsg = (char*)malloc(errorLen);
        isProgram ?
            glGetProgramInfoLog(shader, errorLen, &errorLen, errorMsg)
            : glGetShaderInfoLog(shader, errorLen, &errorLen, errorMsg);
        printf("Shader Log: %d %s \n", errorLen, errorMsg);
        free( errorMsg);
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


void gfx_load_mesh(gfx_mesh * mesh, float * data, float size, float comp)
{
    int comp_size = comp*sizeof(float);
    mesh->data = data;
    mesh->size = size;
    mesh->num_verts = mesh->size/comp_size;

    glUseProgram(_program); //Attaching shaders
    glGenVertexArrays(1, &mesh->vao);
    glBindVertexArray(mesh->vao);

    glGenBuffers(1, &mesh->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);

    glBufferData(GL_ARRAY_BUFFER, mesh->size, mesh->data, GL_STATIC_DRAW);
    glVertexAttribPointer(0, comp, GL_FLOAT, GL_FALSE, comp_size, 0);
    glEnableVertexAttribArray(0);//enabel to draw 
}

void gfx_destroy_mesh(gfx_mesh * mesh)
{
    glUseProgram(_program); //Attaching shaders
    glDeleteVertexArrays(1, &mesh->vao);
    glDeleteBuffers(1, &mesh->vbo);

}
void gfx_render(gfx_mesh * mesh)
{
    glBindVertexArray(mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glDrawArrays(GL_TRIANGLES, 0,  mesh->num_verts);
}


void gfx_bind_texture(gfx_texture * texture)
{
    glBindTexture(GL_TEXTURE_2D, texture->handle);
}


void gfx_load_texture(gfx_texture * texture, byte * data, int width, int height, int comp) 
{ 
    texture->width = width;
    texture->height = height;
    texture->data = data;

    glUseProgram(_program); //Attaching shaders
    glGenTextures(1, &texture->handle);
    glBindTexture(GL_TEXTURE_2D, texture->handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,   GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,   GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,   GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,   GL_NEAREST);
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    switch(comp)
    {
        case 4: 
            texture->format = GL_RGBA;  
        break;
        case 3: 
            texture->format = GL_RGB;  
        break;
        case 2: 
            texture->format = GL_RG;
        break;
        case 1: 
            texture->format = GL_RED;
        break;
        default:
            error("Update gfx_texture : invalid component value (%d)",comp);
        break;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, width, 0,texture->format, GL_UNSIGNED_BYTE, data);
}

void gfx_update_texture(gfx_texture * texture, int x, int y, int width, int height)
{
    glBindTexture(GL_TEXTURE_2D, texture->handle);
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0,x,y,width,height,texture->format, GL_UNSIGNED_BYTE, texture->data);

}

void gfx_set_uniform(const char * name, float value)
{
    int location = glGetUniformLocation(_program, name);
    if(location == -1) 
            error("Could not set uniform (%s)",name);
    glUniform1f(location, value); 
}