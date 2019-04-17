#ifndef NB_KEYS_H
#define NB_KEYS_H
#define NB_KEY_COUNT 512

#include "nb_defs.h"

typedef enum nb_keystate
{
    NB_KEYUP = 0,    //key is not down
    NB_KEYPRESS,     //key just pressed down
    NB_KEYDOWN,     // key is down 
}nb_keystate;

//Key codes as they appear in keymap. One to one with sdl2 keysyms

typedef enum keycode
{
	NB_UNKNOWN     = 0 ,                   
	NB_BACKSPACE     ,                    
	NB_TAB           ,            
	NB_RETURN        ,                  
	NB_ESCAPE        ,                
	NB_SPACE         ,             
	NB_EXCLAIM       ,                 
	NB_QUOTEDBL      ,               
	NB_HASH          ,                 
	NB_DOLLAR        ,                
	NB_PERCENT       ,                
	NB_AMPERSAND     ,                   
	NB_QUOTE         ,              
	NB_LEFTPAREN     ,                   
	NB_RIGHTPAREN    ,                      
	NB_ASTERISK      ,                   
	NB_PLUS          ,               
	NB_COMMA         ,                
	NB_MINUS         ,              
	NB_PERIOD        ,                  
	NB_SLASH         ,            
	NB_0             ,             
	NB_1             ,        
	NB_2             ,             
	NB_3             ,        
	NB_4             ,             
	NB_5             ,        
	NB_6             ,             
	NB_7             ,        
	NB_8             ,             
	NB_9             ,        
	NB_COLON         ,                 
	NB_SEMICOLON     ,                
	NB_LESS          ,                 
	NB_EQUALS        ,                
	NB_GREATER       ,                
	NB_QUESTION      ,                   
	NB_AT            ,          
	NB_LEFTBRACKET   ,                  
	NB_BACKSLASH     ,                     
	NB_RIGHTBRACKET  ,                    
	NB_CARET         ,                 
	NB_UNDERSCORE    ,                   
	NB_BACKQUOTE     ,                   
	NB_a             ,          
	NB_b             ,           
	NB_c             ,          
	NB_d             ,           
	NB_e             ,          
	NB_f             ,           
	NB_g             ,          
	NB_h             ,           
	NB_i             ,          
	NB_j             ,           
	NB_k             ,          
	NB_l             ,           
	NB_m             ,          
	NB_n             ,           
	NB_o             ,          
	NB_p             ,           
	NB_q             ,          
	NB_r             ,           
	NB_s             ,          
	NB_t             ,           
	NB_u             ,          
	NB_v             ,           
	NB_w             ,          
	NB_x             ,           
	NB_y             ,          
	NB_z             ,           
	NB_DELETE     
}nb_keycode;


nb_keystate nb_get_keystate(nb_keycode key);

//native key code mappings. To be used in gfx module. should not be used in main api
nb_keystate nb_get_native_keystate(u16  native_keycode);
void nb_set_native_keystate(u16 native_keycode, nb_keystate state);



#endif
