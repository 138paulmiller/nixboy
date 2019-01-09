#include <GL/glew.h>
#include <SDL2/SDL.h>
#include "glutil.h"

SDL_Window*     _sdl_window;
SDL_Event*      _sdl_event;
SDL_GLContext   _sdl_gl_context;

int _vert_shader, _frag_shader, _program;

#define error(msg, ...) {printf(msg,__VA_ARGS__);exit(-1);}


//map all indices from 1d to 2d using size of palette, sheet or map
// x = i/w
// y = i%w



int check_error(int  shader, int  flag, int isProgram)//////////////////remove eventually
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


int create_shader_stage(uint type, const char* source)
{
    int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, 0);
    glCompileShader(shader);
    check_error(shader, GL_COMPILE_STATUS,0);
    return shader;

}



void destroy_shader()
{
    glDeleteShader(_vert_shader); // removes
    glDeleteShader(_frag_shader); // removes
    glDeleteProgram(_program);

}





/////////////////////////////////////// Begin API ///////////////////////////////////////////////////


void gl_init(const char * title, int width, int height)
{
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0){
        puts("SDL Failed to initialize!"); exit(0);     
    }        
    
    _sdl_window = SDL_CreateWindow(title,SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    width, height, SDL_WINDOW_OPENGL);
    //Initialize opengl color attributes buffer size
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 2);

    _sdl_gl_context = SDL_GL_CreateContext(_sdl_window);
    int status = glewInit(); 
    if(status != GLEW_OK)
    {
        error("GLEW Failed to initialize %d", status);
        
    }   
}


void gl_destroy()
{
    destroy_shader(); 

    SDL_DestroyWindow(_sdl_window);
    SDL_GL_DeleteContext(_sdl_gl_context);
    SDL_Quit();
}

void gl_clear()
{
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


int gl_update()
{
  //swap render and display buffers
    SDL_GL_SwapWindow(_sdl_window);
    //Poll input events
    SDL_Event event;
    while (SDL_PollEvent(&event))
        if (event.type == SDL_QUIT) return 0;
    return 1;

    gl_clear();
    glUseProgram(_program); //Attaching shaders
}


void gl_load_shader(const char * vertex_source, const char * fragment_source)
{
    _program = glCreateProgram();
    puts("Vert...");
    glAttachShader(_program, create_shader_stage(GL_VERTEX_SHADER, vertex_source) );
    puts("Frag....");
    glAttachShader(_program, create_shader_stage(GL_FRAGMENT_SHADER,fragment_source) );
    glLinkProgram(_program);
    if(check_error(_program, GL_LINK_STATUS, 1))
        return;
    glValidateProgram(_program);
    if(check_error(_program, GL_VALIDATE_STATUS, 1))return;
}


void gl_load_mesh(mesh * obj, float * data, float size, float comp)
{
    int comp_size = comp*sizeof(float);
    obj->data = data;
    obj->size = size;
    obj->num_verts = obj->size/comp_size;

    glUseProgram(_program); //Attaching shaders
    glGenVertexArrays(1, &obj->vao);
    glBindVertexArray(obj->vao);

    glGenBuffers(1, &obj->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, obj->vbo);

    glBufferData(GL_ARRAY_BUFFER, obj->size, obj->data, GL_STATIC_DRAW);
    glVertexAttribPointer(0, comp, GL_FLOAT, GL_FALSE, comp_size, 0);
    glEnableVertexAttribArray(0);//enabel to draw 
}

void gl_mesh_destroy(mesh * obj)
{
    glUseProgram(_program); //Attaching shaders
    glDeleteVertexArrays(1, &obj->vao);
    glDeleteBuffers(1, &obj->vbo);

}
void gl_render(mesh * obj)
{
    glBindVertexArray(obj->vao);
    glBindBuffer(GL_ARRAY_BUFFER, obj->vbo);
    glDrawArrays(GL_TRIANGLES, 0,  obj->num_verts);
}


void gl_bind_texture(texture * obj)
{
    glBindTexture(GL_TEXTURE_2D, obj->handle);
}

#define comp_to_format(comp) (comp == 4 ?           \
                        GL_RGBA :                  \
                        (comp == 3 ?                \
                            GL_RGB :                \
                            (comp == 1 ?            \
                                GL_RED : -1)));    \


void gl_load_texture(texture * obj, color * data, int width, int height, int comp) 
{ 
    obj->width = width;
    obj->height = height;
    obj->data = data;
    obj->comp = comp;

    int format= comp_to_format(obj->comp);
    if(format == -1) 
            error("Update Texture : invalid component value (%d)",comp);
    glUseProgram(_program); //Attaching shaders
    glGenTextures(1, &obj->handle);
    glBindTexture(GL_TEXTURE_2D, obj->handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,   GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,   GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,   GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,   GL_NEAREST);
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, width, 0,format, GL_UNSIGNED_BYTE, (byte*)data);
}

void gl_update_texture(texture * obj, int x, int y, int width, int height)
{
    int format= comp_to_format(obj->comp);
    if(format == -1) 
            error("Update Texture : invalid component value (%d)",obj->comp);
    glBindTexture(GL_TEXTURE_2D, obj->handle);
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, obj->width, obj->height, 0, format, GL_UNSIGNED_BYTE, obj->data);

    glTexSubImage2D(GL_TEXTURE_2D, 0,x,y,width,height,format,GL_UNSIGNED_BYTE, (byte*)obj->data);

}

void gl_set_uniform(const char * name, float value)
{
    int location = glGetUniformLocation(_program, name);
    if(location == -1) 
            error("Could not set uniform (%s)",name);
    glUniform1f(location, value); 
}