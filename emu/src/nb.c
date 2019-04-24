#include "nb.h"

//Statics that represent the nixboy machine
static nb_state     _nb;

    
//-----Utilities

// These should not modify the above statics


//Log nb_error to crash report.txt and call shutdown
//add after error nb_log(__VA_ARGS__)     \

#define fatal_error(...){ \
nb_error(__VA_ARGS__)     \
nb_shutdown();            \
                          \
}                       


static void _load_shader(nb_shader * shader,  const str vert_filepath, const str frag_filepath)
{
    //create file struct, intermittengly check last modified data, if changes then reload (hotload ) 
    str vert_source = 0;
    str frag_source = 0 ;
    int vert_size;
    int frag_size;

    nb_debug("Loading Shader : %s  %s\n", vert_filepath, frag_filepath)
    

    nb_fread(vert_filepath, &vert_source, &vert_size); 
    nb_fread(frag_filepath, &frag_source, &frag_size);

    if(! vert_source)
    {
        nb_error("Failed to Read res/sprite.vert file\n")
        exit(1);
    }   
    
    if(! frag_source)
    {

        nb_error("Failed to Read res/sprite.frag file\n")
        exit(1);
    }   

    nb_debug("Compiling Shader\n");

    nb_init_shader(shader, vert_source, frag_source);
    if(vert_source)
        nb_free(vert_source);
    if(frag_source)
        nb_free(frag_source);

    nb_debug("Done\n");
}


void _bind_sprite_shader()
{
    nb_bind_shader(&_nb.gfx.sprite_shader);
    nb_use_atlas  (&_nb.gfx.sprite_atlas);
    nb_use_palette(&_nb.gfx.palette);



}

void _update_sprite_shader_cache()
{

    //set indeices for texture units        
    nb_set_uniform_int(&_nb.gfx.sprite_shader   , NB_UNIFORM_ATLAS     ,    NB_TEXTURE_UNIT_ATLAS      );
    nb_set_uniform_int(&_nb.gfx.sprite_shader   , NB_UNIFORM_PALETTE   ,    NB_TEXTURE_UNIT_PALETTE    );

    nb_set_uniform_vec2i(&_nb.gfx.sprite_shader , NB_UNIFORM_SCREEN_RESOLUTION  ,  &_nb.cache.screen_resolution       );
    nb_set_uniform_vec2i(&_nb.gfx.sprite_shader , NB_UNIFORM_ATLAS_RESOLUTION   ,  &_nb.cache.sprite_atlas_resolution );
    
    nb_set_uniform_int(&_nb.gfx.sprite_shader   , NB_UNIFORM_SCREEN_SCALE        ,  _nb.cache.screen_scale);
    nb_set_uniform_int(&_nb.gfx.sprite_shader   , NB_UNIFORM_PALETTE_SIZE        ,  _nb.cache.palette_size );
    nb_set_uniform_int(&_nb.gfx.sprite_shader   , NB_UNIFORM_COLOR_DEPTH         ,  _nb.cache.color_depth);
    //draw each sprite and draw!

 
}


void _load_cache(nb_settings * settings)
{
    _nb.cache.screen_resolution.x = settings->screen.width;
    _nb.cache.screen_resolution.y = settings->screen.height;
    
    _nb.cache.sprite_atlas_resolution.x = settings->gfx.sprite_atlas_width;
    _nb.cache.sprite_atlas_resolution.y = settings->gfx.sprite_atlas_height; 

    _nb.cache.tile_atlas_resolution.x = settings->gfx.tile_atlas_width;
    _nb.cache.tile_atlas_resolution.y = settings->gfx.tile_atlas_height; 


    _nb.cache.sprite_size       = settings->gfx.sprite_size; 
    _nb.cache.sprite_table_size = settings->gfx.max_sprite_count;
    
    _nb.cache.screen_scale  = settings->screen.scale;
    _nb.cache.color_depth   = settings->gfx.color_depth;
    _nb.cache.palette_size  = settings->gfx.palette_size;
    //allocate buffers for palette, atlases sprites,, etc...
    
    ////////////////////////// Initialize Cache /////////////////////////////
    _nb.cache.sprite_table_block_size   =   _nb.cache.sprite_table_size * sizeof(nb_sprite) ;
    _nb.cache.palette_block_size        =   _nb.cache.palette_size      * sizeof(rgb)       ;
    _nb.cache.sprite_atlas_block_size   = sizeof(byte) 
                                    * _nb.cache.sprite_atlas_resolution.x 
                                    * _nb.cache.sprite_atlas_resolution.y;
    

    _nb.cache.tile_atlas_block_size   = sizeof(byte) 
                                    * _nb.cache.tile_atlas_resolution.x 
                                    * _nb.cache.tile_atlas_resolution.y;

}
void _load_ram()
{
    _nb.ram.sprite_table        = nb_malloc(_nb.cache.sprite_table_block_size);
    memset(_nb.ram.sprite_table, 0, _nb.cache.sprite_table_block_size );

    _nb.ram.palette_colors         = nb_malloc(_nb.cache.palette_block_size);
    memset(_nb.ram.palette_colors, 0, _nb.cache.palette_block_size );

    _nb.ram.sprite_atlas_indices    = nb_malloc(_nb.cache.sprite_atlas_block_size);
    memset(_nb.ram.sprite_atlas_indices, 0, _nb.cache.sprite_atlas_block_size );
}

void _load_gfx()
{

    nb_init_palette( &_nb.gfx.palette, _nb.ram.palette_colors,_nb.cache.palette_size);
    nb_init_atlas(
        &_nb.gfx.sprite_atlas, 
        _nb.ram.sprite_atlas_indices, 
        _nb.cache.sprite_atlas_resolution.x , 
        _nb.cache.sprite_atlas_resolution.y );
}


//---------------------- nb api --------------------------------

void        nb_startup(nb_settings * settings)
{

    if( ! settings)
    {
        fatal_error("Failed to Initalize nixboy. null settings!")
    }
    u32 scale  = settings->screen.scale;
    u32 width  = settings->screen.width;
    u32 height = settings->screen.height;

    nb_init_window(settings->screen.title, width*scale, height*scale );
    

    ////////////////////////// Load Uniforms Settings /////////////////////////////
    _load_cache(settings);    

    _load_ram();
    
    //  ---------------------- Compile Shaders -------------------
    _load_shader(&_nb.gfx.sprite_shader, "res/sprite.vert","res/sprite.frag");
 //   _load_shader(&_nb.gfx.tile_shader, "res/tile.vert","res/tile.frag");

    _bind_sprite_shader();
    _update_sprite_shader_cache();
    _load_gfx();   
}

nb_status   nb_update()
{
    static nb_status status;
    status =  nb_update_window();
    switch(status)
    {
        case NB_FAILURE:
        break;
        case NB_PAUSE:  
        break;
        case NB_QUIT:   
        break;

        default:
            status = NB_CONTINUE; 
        break;
    }

    return status;
}

nb_status   nb_draw(u32 flags)
{


        //sprite0.offset.x= sprite0.offset.x%(int)(sqrt(NB_ATLAS_SIZE));
        
//        nb_set_sprite_xy(&sprite0, x, y);
    static int sprite_index ;
    static nb_sprite * sprite;
     _bind_sprite_shader();

    //update an
    if( nb_test(flags, NB_FLAG_PALETTE_DIRTY))
    {
        puts("asdasd");
        nb_update_palette(&_nb.gfx.palette);
    }
    //update an
    if( nb_test(flags, NB_FLAG_SPRITE_ATLAS_DIRTY))
    {
        puts("asdasd");
        nb_update_atlas(&_nb.gfx.sprite_atlas);
    }
    _update_sprite_shader_cache();


    //  ---------------------------- draw tilemap -------------------


      //---------------------- draw sprites --------------------------------
    sprite_index  = 0;
    for(sprite_index=0; sprite_index    < _nb.cache.sprite_table_size; sprite_index++)
    {

        sprite = &_nb.ram.sprite_table[sprite_index];
        //may or may not render. if destroyed. rednere will skip
        
        nb_render_sprite(sprite);   
    } 
}


void        nb_shutdown()
{

    //delete sprites
    nb_free(_nb.ram.sprite_table);
    _nb.ram.sprite_table = 0;

    //free up data
    if( _nb.ram.palette_colors)
    {
        nb_free(_nb.ram.palette_colors);
        _nb.ram.palette_colors = 0;
    } 

    if( _nb.ram.sprite_atlas_indices)
    {
        nb_free(_nb.ram.sprite_atlas_indices);
        _nb.ram.sprite_atlas_indices = 0;
    } 

    nb_destroy_palette( & _nb.gfx.palette      );
    nb_destroy_atlas  ( & _nb.gfx.tile_atlas   );
    nb_destroy_atlas  ( & _nb.gfx.sprite_atlas );
    
    //destroy shader 
    nb_destroy_shader( & _nb.gfx.sprite_shader );
    nb_destroy_shader( & _nb.gfx.tile_shader   );

    nb_destroy_window(); 

    exit(0);
}
// --------------------------------- Accessors / Mutators for intoernal structure --------------
void        nb_set_palette(rgb * colors)
{
    memcpy(_nb.ram.palette_colors, colors, _nb.cache.palette_block_size);
    nb_update_palette( &_nb.gfx.palette);
}

rgb *        nb_get_palette()
{
    return _nb.ram.palette_colors;
}


void        nb_set_sprite_atlas(byte * indices)
{
    memcpy(_nb.ram.sprite_atlas_indices, indices,  _nb.cache.sprite_atlas_block_size);
    nb_update_atlas(&_nb.gfx.sprite_atlas);
}

byte *        nb_get_sprite_atlas()
{
    return  _nb.ram.sprite_atlas_indices;
}


nb_sprite *  nb_add_sprite( nb_sprite_type type,int index)
{

    vec2i offset  = { index * _nb.cache.sprite_size , 0};
    vec2f size    = { 0,  0};
    nb_sprite * sprite = &_nb.ram.sprite_table[index];

    switch(type)
    {
        //Regular 8x8
        case NB_SPRITE_REGULAR:
            size.x  = _nb.cache.sprite_size;
            size.y  = _nb.cache.sprite_size;
        break;
        //Tall 8x16    
        case NB_SPRITE_WIDE:
            size.x = _nb.cache.sprite_size * 2 ;
            size.y = _nb.cache.sprite_size;
        break;
         //Wide 16x8
        case NB_SPRITE_TALL:
            size.x = _nb.cache.sprite_size;
            size.y = _nb.cache.sprite_size * 2 ;
        break;
    }


    nb_init_sprite(sprite, &_nb.gfx.sprite_shader, offset, size);

    return  sprite;
}

void        nb_remove_sprite(nb_sprite * sprite)
{
    nb_destroy_sprite(sprite); //set sheet to null, no longer renderable    
}


