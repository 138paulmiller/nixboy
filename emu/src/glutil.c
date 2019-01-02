#include <GL/glew.h>
#include <SDL2/SDL.h>
#include "glutil.h"

SDL_Window*     _sdl_window;
SDL_Event*      _sdl_event;
SDL_GLContext   _sdl_gl_context;

int _texture, _vert_shader, _frag_shader, _program;
int _vao, _vbo;

#define error(msg, ...) {printf(msg,__VA_ARGS__);exit(-1);}

static float verts[12] = {
    0, 0,
    0, 1,
    1, 1,

    0, 0,
    1, 0,
    1, 1,
};
//map all indices from 1d to 2d using size of palette, sheet or map
// x = i/w
// y = i%w

static const char * _vertex_source = 
    "#version 130\n"
    //"uniform vec2 size;"
    "in vec2 pos;"
    "out vec2 uv;"
    "void main(){"
        "uv=pos;"
        "gl_Position  =vec4(pos*2-1,1,1);\n"

    "}";


const char * _fragment_source = 
    "#version 130\n"
    "uniform sampler2D sampler;"
    "in vec2 uv;"
    "out vec4 color;"
    "void main(){\n"
        "color = texture(sampler,uv);\n"
    "}";


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

void load_shader()
{
    _program = glCreateProgram();
    puts("Vert...");
    glAttachShader(_program, create_shader_stage(GL_VERTEX_SHADER, _vertex_source) );
    puts("Frag....");
    glAttachShader(_program, create_shader_stage(GL_FRAGMENT_SHADER,_fragment_source) );
    glLinkProgram(_program);
    if(check_error(_program, GL_LINK_STATUS, 1))
        return;
    glValidateProgram(_program);
    if(check_error(_program, GL_VALIDATE_STATUS, 1))return;
}



void destroy_shader()
{
    glDeleteShader(_frag_shader); // removes
    glDeleteProgram(_program);

}

void create_vertex_array()
{
    glUseProgram(_program); //Attaching shaders
    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), 0);
    glEnableVertexAttribArray(0);//enabel to draw
  
}



void destroy_vertex_array()
{
    glDeleteVertexArrays(1, &_vao);
    glDeleteBuffers(1, &_vbo);

}

//bind to use
void draw_vertex_array() 
{
    //bind the VAO and its element buffer
    //glActiveTexture(GL_TEXTURE0); 
    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glDrawArrays(GL_TRIANGLES, 0,  6);
}


/////////////////////////////////////// Begin API ///////////////////////////////////////////////////


void gl_init(int width, int height)
{
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0){
        puts("SDL Failed to initialize!"); exit(0);     
    }        
    
    _sdl_window = SDL_CreateWindow("cccc",SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
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

    load_shader(); 
    create_vertex_array(); 
}


void gl_destroy()
{
    destroy_shader(); 
    destroy_vertex_array();

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
void gl_render()
{
    draw_vertex_array();
}
#define gl_format(comp) (comp == 3 ? GL_RGB : (comp == 1 ? GL_RED : -1)); 
void gl_load_texture(int * texture, byte * data, int x, int y, int w, int h, int comp) 
{ 
    int format= gl_format(comp);
    if(format == -1) 
            error("Update Texture : invalid component value (%d)",comp);
       glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,   GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,   GL_NEAREST);
    //glPixelStorei( GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, format, GL_UNSIGNED_BYTE, data);
    
}

void gl_update_texture(int texture, byte * data, int x, int y, int w, int h,int comp)
{
    int format= gl_format(comp);
    if(format == -1) 
            error("Update Texture : invalid component value (%d)",comp);
    glBindTexture(GL_TEXTURE_2D, texture);
    //glPixelStorei( GL_UNPACK_ALIGNMENT, 1);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glTexSubImage2D(GL_TEXTURE_2D, 0,
                    x,y,w,h,
                    format,
                    GL_UNSIGNED_BYTE, 
                    data);
}




int gl_test()
{
    int width = 256, height = 256;
    gl_init(width, height);

    int comp = 1;
    int len = width * height*comp;
    byte * data = (byte*)malloc(len);
    //memset(&texture[0], 120, len);
    int texture;
    int i,j,x;
    for(j=0; j < height; j++)
        for(i=0; i < width; i++)
        {
            x = comp*(j*height+i);
            data[x++] =  i;
            //data[x++] =  j;
            //data[x] =  0;
        }
    gl_load_texture(&texture, &data[0], 0,0,width,height, comp); 
    
    while(gl_update())
    {
        gl_update_texture(texture, &data[0],0,0, width,height, comp); 
        gl_render();
    }   
    gl_destroy(); 
    return 0;
}

