#include "nb_gfx.h"

//
#include <GL/glew.h>
#include <SDL2/SDL.h>

//Global State
static SDL_Window*     _sdl_window;
static SDL_Event*      _sdl_event;
static SDL_GLContext   _sdl_gfx_context;
static gfx_shader *     _active_shader;  //currently bound shader

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
        printf("\nShader Log: %d %s \n", error_len, error_msg);
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
  //swap render and display buffers
    SDL_GL_SwapWindow(_sdl_window);
    //Poll input events
    SDL_Event event;
    while (SDL_PollEvent(&event))
        if (event.type == SDL_QUIT) return 0;
    return 1;

    gfx_clear();
}


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
    glValidateProgram(shader->program);
    if(_check_shader_error(shader->program, GL_VALIDATE_STATUS, 1) == GFX_FAILURE){
        return GFX_FAILURE;
    }
    
    shader->pos_loc = glGetAttribLocation(shader->program, GFX_ATTRIB_POS);
    shader->uv_loc = glGetAttribLocation(shader->program, GFX_ATTRIB_UV);
    if(shader->pos_loc == -1)
    {
        nb_error("GFX Invalid name for Position Attrib: Expected %s", GFX_ATTRIB_POS);
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

void    gfx_bind_shader(gfx_shader * shader)
{
    _active_shader = shader;
    glUseProgram(shader->program); //Attaching shaders
}
gfx_status    gfx_set_uniform(gfx_shader * shader, const char * name, float value)
{
    int location = glGetUniformLocation(shader->program, name);
    if(location == -1) 
    {
        nb_error("Could not set uniform (%s)",name);
        return GFX_FAILURE;
    }   
    glUniform1f(location, value); 
    return GFX_SUCCESS;
}



gfx_status gfx_init_mesh(gfx_mesh * mesh, gfx_shader * shader, gfx_vertex *  verts, int num_verts)
{
    int comp_size = sizeof(gfx_vertex);
    int stride = 4*sizeof(float);
    int size = sizeof(gfx_vertex)*num_verts;        

    mesh->verts = verts;
    mesh->num_verts = num_verts;
    mesh->shader = shader;
    glUseProgram(shader->program); //Attaching shaders

    glGenVertexArrays(1, &mesh->vao);
    glBindVertexArray(mesh->vao);

    glGenBuffers(1, &mesh->vbo);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, size, verts, GL_STATIC_DRAW);

    glVertexAttribPointer(shader->pos_loc, 2, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(shader->pos_loc);//enable to draw
    
    glVertexAttribPointer(shader->uv_loc, 2, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(gfx_vertex, uv));
    glEnableVertexAttribArray(shader->uv_loc);//enable to draw
    glUseProgram(0); //Attaching shaders

    return GFX_SUCCESS;
}

void gfx_destroy_mesh(gfx_mesh * mesh)
{
    glUseProgram(mesh->shader->program); //Attaching shaders
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


void gfx_bind_texture(gfx_texture * texture)
{
    glBindTexture(texture->type, texture->handle);
}


gfx_status  gfx_init_texture(gfx_texture * texture, gfx_texture_type type, gfx_format format, byte  * data, int width, int height)
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
            nb_error("GFX: load texture : invalid format value (%d)",format);
            return GFX_FAILURE;
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
            nb_error("GFX: load texture : invalid format type (%d)",type);
            return GFX_FAILURE;
        break;
    }
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
    return GFX_SUCCESS;
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


gfx_status  gfx_init_rect(gfx_rect * rect, gfx_shader * shader, int x, int y, int w, int h)
{
 
    gfx_vertex * verts = nb_malloc(6*sizeof(gfx_vertex));
    //tri 1
    
    verts[0].pos[0] =x;    verts[0].pos[1] =y;
    verts[0].uv[0]  =0;    verts[0].uv[1]  =0;

    verts[1].pos[0] =x;    verts[1].pos[1] =y+h;
    verts[1].uv[0]  =0;    verts[1].uv[1]  =1;

    verts[2].pos[0] =x+w;  verts[2].pos[1] =y+h;
    verts[2].uv[0]  =1;    verts[2].uv[1]  =1;
    //tri 2    
    verts[3].pos[0] =x;    verts[3].pos[1] =y;
    verts[3].uv[0]  =0;    verts[3].uv[1]  =0;

    verts[4].pos[0] =x+w;  verts[4].pos[1] =y;
    verts[4].uv[0]  =1;    verts[4].uv[1]  =0;

    verts[5].pos[0] =x+w;  verts[5].pos[1] =y+h;
    verts[5].uv[0]  =1;    verts[5].uv[1]  =1;
    
    rect->mesh.verts = verts;
    return gfx_init_mesh(&rect->mesh, shader, rect->mesh.verts, 6);
}


void  gfx_destroy_rect(gfx_rect * rect)
{
    free(rect->mesh.verts);
    gfx_destroy_mesh(&rect->mesh);
}
void  gfx_render_rect(gfx_rect * rect)
{
    gfx_render_mesh(&rect->mesh);   
}