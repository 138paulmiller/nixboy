#include "nb_keys.h"
#include <SDL2/SDL.h>

//native keycodes map to this index. Uses get_keycode to map 
static nb_keystate     _keystates[NB_KEY_COUNT ] = {NB_KEYUP };
//translates from internel keycode to exposed nb_key codes

static u32 _nb_to_sdl_keymap[NB_KEY_COUNT] = { 
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


nb_keystate nb_get_keystate(nb_keycode key)
{
    //map from nb_key to native sdl
    return _keystates[_nb_to_sdl_keymap[ key]];
}

void nb_set_native_keystate(u16 native_keycode, nb_keystate state)
{
    _keystates[native_keycode] = state;
}
nb_keystate nb_get_native_keystate(u16 native_keycode)
{
    return _keystates[native_keycode];
}