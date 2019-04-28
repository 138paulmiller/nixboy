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
        nb_error("Failed to Read %s file\n", vert_filepath);
        exit(1);
    }   
    
    if(! frag_source)
    {

        nb_error("Failed to Read %s file\n", frag_filepath);
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

void _bind_shader(nb_shader_index shader_index, bool update_cache)
{
    //not thread safe
    static nb_shader * shader;
    shader = &_nb.gfx.shaders[shader_index];
    nb_bind_shader(shader);
    switch(shader_index)
    {
        case NB_SPRITE_SHADER:
        {
            nb_use_palette(&_nb.gfx.sprite_palette);
            nb_use_atlas  (&_nb.gfx.sprite_atlas);
            //set indeices for texture units        
            if(update_cache)
            {

                nb_set_uniform_int(shader   , NB_UNIFORM_ATLAS     ,    NB_TEXTURE_UNIT_ATLAS      );
                nb_set_uniform_int(shader   , NB_UNIFORM_PALETTE   ,    NB_TEXTURE_UNIT_PALETTE    );

                nb_set_uniform_vec2i(shader, NB_UNIFORM_SCREEN_RESOLUTION  ,  &_nb.cache.screen_resolution       );
                nb_set_uniform_vec2i(shader, NB_UNIFORM_ATLAS_RESOLUTION   ,  &_nb.cache.sprite_atlas_resolution );
                
                nb_set_uniform_int(shader   , NB_UNIFORM_SCREEN_SCALE        ,  _nb.cache.screen_scale);
                nb_set_uniform_int(shader   , NB_UNIFORM_PALETTE_SIZE        ,  _nb.cache.palette_size );
                nb_set_uniform_int(shader   , NB_UNIFORM_COLOR_DEPTH         ,  _nb.cache.color_depth);    
            }
        }
        break;
        case NB_TILE_SHADER:
        {
            nb_use_palette(&_nb.gfx.tile_palette);
            nb_use_atlas  (&_nb.gfx.tile_atlas);
            if(update_cache)
            {
                //set indeices for texture units        
                nb_set_uniform_int(shader   , NB_UNIFORM_ATLAS     ,    NB_TEXTURE_UNIT_ATLAS      );
                nb_set_uniform_int(shader   , NB_UNIFORM_PALETTE   ,    NB_TEXTURE_UNIT_PALETTE    );
                nb_set_uniform_int(shader   , NB_UNIFORM_TILEMAP   ,    NB_TEXTURE_UNIT_TILEMAP    );

                nb_set_uniform_vec2i(shader, NB_UNIFORM_SCREEN_RESOLUTION  ,  &_nb.cache.screen_resolution       );
                nb_set_uniform_vec2i(shader, NB_UNIFORM_ATLAS_RESOLUTION   ,  &_nb.cache.tile_atlas_resolution );
                
                nb_set_uniform_int(shader   , NB_UNIFORM_SCREEN_SCALE        ,  _nb.cache.screen_scale);
                nb_set_uniform_int(shader   , NB_UNIFORM_PALETTE_SIZE        ,  _nb.cache.palette_size );
                nb_set_uniform_int(shader   , NB_UNIFORM_COLOR_DEPTH         ,  _nb.cache.color_depth);
            }
        }
        break;
    

    }
}

void _load_cache(nb_settings * settings)
{
    _nb.cache.screen_resolution.x = settings->screen.width;
    _nb.cache.screen_resolution.y = settings->screen.height;
    _nb.cache.screen_scale        = settings->screen.scale;
    
    _nb.cache.color_depth             = settings->gfx.color_depth;
    _nb.cache.palette_size            = settings->gfx.palette_size;
    
    _nb.cache.sprite_atlas_resolution.x = settings->gfx.sprite_atlas_width;
    _nb.cache.sprite_atlas_resolution.y = settings->gfx.sprite_atlas_height; 
    _nb.cache.sprite_resolution.x       = settings->gfx.sprite_width;
    _nb.cache.sprite_resolution.y       = settings->gfx.sprite_height; 
    _nb.cache.sprite_table_size         = settings->gfx.max_sprite_count;

    _nb.cache.tile_atlas_resolution.x = settings->gfx.tile_atlas_width;
    _nb.cache.tile_atlas_resolution.y = settings->gfx.tile_atlas_height; 
    _nb.cache.tile_resolution.x       = settings->gfx.tile_width;
    _nb.cache.tile_resolution.y       = settings->gfx.tile_height; 

    _nb.cache.tilemap_resolution.x       = settings->gfx.tilemap_width;
    _nb.cache.tilemap_resolution.y       = settings->gfx.tilemap_height; 


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

    _nb.cache.tilemap_block_size   = sizeof(byte) 
                                    * _nb.cache.tilemap_resolution.x 
                                    * _nb.cache.tilemap_resolution.y;

}



void _load_ram()
{
    _nb.ram.sprite_table           = nb_malloc(_nb.cache.sprite_table_block_size );
    _nb.ram.tile_palette_colors    = nb_malloc(_nb.cache.palette_block_size      );
    _nb.ram.sprite_palette_colors  = nb_malloc(_nb.cache.palette_block_size      );
    _nb.ram.sprite_atlas_indices   = nb_malloc(_nb.cache.sprite_atlas_block_size );
    _nb.ram.tile_atlas_indices     = nb_malloc(_nb.cache.tile_atlas_block_size   );
    _nb.ram.tilemap_indices          = nb_malloc(_nb.cache.tilemap_block_size   );


    memset(_nb.ram.sprite_table,          0, _nb.cache.sprite_table_block_size  );
    memset(_nb.ram.tile_palette_colors,   0, _nb.cache.palette_block_size       );
    memset(_nb.ram.sprite_palette_colors, 0, _nb.cache.palette_block_size       );
    memset(_nb.ram.sprite_atlas_indices,  0, _nb.cache.sprite_atlas_block_size  );
    memset(_nb.ram.tile_atlas_indices,    0, _nb.cache.tile_atlas_block_size    );
    memset(_nb.ram.tilemap_indices,         0, _nb.cache.tilemap_block_size    );

}

void _load_gfx()
{
    nb_debug("Initializing GFX Modules\n");
    _bind_shader(NB_SPRITE_SHADER, 0);

    nb_init_palette( &_nb.gfx.sprite_palette, _nb.ram.sprite_palette_colors,_nb.cache.palette_size);
    nb_init_atlas(
        &_nb.gfx.sprite_atlas, 
        _nb.ram.sprite_atlas_indices, 
        _nb.cache.sprite_atlas_resolution.x , 
        _nb.cache.sprite_atlas_resolution.y );


    nb_debug("Initializing Tile Atlas\n");
    _bind_shader(NB_TILE_SHADER, 0);
    
    nb_init_palette( 
        &_nb.gfx.tile_palette, 
        _nb.ram.tile_palette_colors,
        _nb.cache.palette_size);
    
    nb_init_atlas(
        &_nb.gfx.tile_atlas, 
        _nb.ram.tile_atlas_indices, 
        _nb.cache.tile_atlas_resolution.x , 
        _nb.cache.tile_atlas_resolution.y );

    nb_init_tilemap(
        &_nb.gfx.tilemap,
        &_nb.gfx.shaders[NB_TILE_SHADER], 
        _nb.ram.tilemap_indices, 
 
        _nb.cache.tile_resolution.x , 
        _nb.cache.tile_resolution.y ,
        _nb.cache.tilemap_resolution.x , 
        _nb.cache.tilemap_resolution.y );
}
#define DUMP_INT(cache_var)     printf(#cache_var": %d\n", _nb.cache.cache_var)
#define DUMP_VEC2(cache_vec2)   printf(#cache_vec2": %d %d\n", _nb.cache.cache_vec2.x,_nb.cache. cache_vec2.y)

void _dump_cache()
{
    printf("\n-----------------------Cache -------------------------\n");
        DUMP_VEC2(sprite_resolution           );
        DUMP_VEC2(screen_resolution           );
        DUMP_VEC2(sprite_atlas_resolution     );
        DUMP_VEC2(tile_atlas_resolution       );    //numbe of tiles along tilemap edge
        DUMP_VEC2(tilemap_resolution            );         //numbe of tiles along tilemap edge
        DUMP_VEC2(tile_resolution             );          //number of pixels along tile edge
        DUMP_INT (sprite_table_size           ); 
        DUMP_INT (palette_size                );            //dwisth of palette in colors
        DUMP_INT (color_depth                 );
        DUMP_INT (screen_scale                );
        DUMP_INT (sprite_table_block_size     );
        DUMP_INT (palette_block_size          );
        DUMP_INT (sprite_atlas_block_size     );
        DUMP_INT (tile_atlas_block_size       );
        DUMP_INT (tilemap_block_size            );

    printf("\n------------------------------------------------------\n");

}
#undef DUMP
#undef DUMP_VEC2
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
    _dump_cache();


    _load_ram();
    
    //  ---------------------- Compile Shaders -------------------
    _load_shader(&_nb.gfx.shaders[NB_SPRITE_SHADER], "res/sprite.vert","res/sprite.frag");
    _load_shader(&_nb.gfx.shaders[NB_TILE_SHADER], "res/tilemap.vert","res/tilemap.frag");

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
//  ---------------------------- handle input -------------------
    
    //update an
    if( nb_test(flags, NB_FLAG_SPRITE_PALETTE_DIRTY))
    {
        nb_update_palette(&_nb.gfx.sprite_palette);
    }
    //update an
    if( nb_test(flags, NB_FLAG_SPRITE_ATLAS_DIRTY))
    {
        nb_update_atlas(&_nb.gfx.sprite_atlas);
    }

    if( nb_test(flags, NB_FLAG_TILE_ATLAS_DIRTY))
    {
        nb_update_atlas(&_nb.gfx.tile_atlas);
    }

    if( nb_test(flags, NB_FLAG_TILE_ATLAS_DIRTY))
    {
        nb_update_tilemap(&_nb.gfx.tilemap);
    }

    //  ---------------------------- draw tilemap -------------------
    _bind_shader(NB_TILE_SHADER, 1);
    nb_render_tilemap(&_nb.gfx.tilemap);   
    /////////////////////////////////
    _bind_shader(NB_SPRITE_SHADER , 1);
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

    // ---------------------- dealloc ram ---------------------
    //delete sprites
    nb_free(_nb.ram.sprite_table);
    _nb.ram.sprite_table = 0;

    //free up data

    if( _nb.ram.sprite_palette_colors)
    {
        nb_free(_nb.ram.sprite_palette_colors);
        _nb.ram.sprite_palette_colors = 0;
    } 

    if( _nb.ram.sprite_atlas_indices)
    {
        nb_free(_nb.ram.sprite_atlas_indices);
        _nb.ram.sprite_atlas_indices = 0;
    } 


    if( _nb.ram.tile_palette_colors)
    {
        nb_free(_nb.ram.tile_palette_colors);
        _nb.ram.tile_palette_colors = 0;
    } 

    if( _nb.ram.tile_atlas_indices)
    {
        nb_free(_nb.ram.tile_atlas_indices);
        _nb.ram.tile_atlas_indices = 0;
    } 

    if( _nb.ram.tilemap_indices)
    {
        nb_free(_nb.ram.tilemap_indices);
        _nb.ram.tilemap_indices = 0;
    } 

    // ------------------------------ destroy gfx module ------------------
    nb_destroy_palette( & _nb.gfx.tile_palette      );
    nb_destroy_palette( & _nb.gfx.sprite_palette      );
    
    nb_destroy_atlas  ( & _nb.gfx.tile_atlas   );
    nb_destroy_atlas  ( & _nb.gfx.sprite_atlas );
    //destroy shader 
    nb_destroy_shader( & _nb.gfx.shaders[NB_SPRITE_SHADER] );
    nb_destroy_shader( & _nb.gfx.shaders[NB_TILE_SHADER]   );

    nb_destroy_window(); 

    exit(0);
}
// --------------------------------- Accessors / Mutators for intoernal structure --------------
void        nb_set_sprite_palette(rgb * colors)
{
    memcpy(_nb.ram.sprite_palette_colors, colors, _nb.cache.palette_block_size);
    nb_update_palette( &_nb.gfx.sprite_palette);
}

rgb *        nb_get_sprite_palette()
{
    return _nb.ram.sprite_palette_colors;
}


void        nb_set_sprite_atlas(byte * color_indices)
{
    memcpy(_nb.ram.sprite_atlas_indices, color_indices,  _nb.cache.sprite_atlas_block_size);
    nb_update_atlas(&_nb.gfx.sprite_atlas);
}

byte *        nb_get_sprite_atlas()
{
    return  _nb.ram.sprite_atlas_indices;
}

void        nb_set_tile_palette(rgb * colors)
{
    memcpy(_nb.ram.tile_palette_colors, colors, _nb.cache.palette_block_size);
    nb_update_palette( &_nb.gfx.tile_palette);
}

rgb *        nb_get_tile_palette()
{
    return _nb.ram.tile_palette_colors;
}


void        nb_set_tile_atlas(byte * color_indices)
{
    memcpy(_nb.ram.tile_atlas_indices, color_indices,  _nb.cache.tile_atlas_block_size);
    nb_update_atlas(&_nb.gfx.tile_atlas);
}

byte *        nb_get_tile_atlas()
{
    return  _nb.ram.sprite_atlas_indices;
}

void        nb_set_tilemap(byte * tilemap_indices)
{
    memcpy(_nb.ram.tilemap_indices, tilemap_indices,  _nb.cache.tilemap_block_size);
    nb_update_tilemap(&_nb.gfx.tilemap);
}

byte *        nb_get_tilemap()
{
    return  _nb.ram.tilemap_indices;
}


nb_sprite *  nb_add_sprite( nb_sprite_type type, int index)
{
    //always stride across min to snap all to regular grid
    u16 min_dimension ;
    vec2i offset  = { 0, 0};
    vec2f size    = { 0,  0};
    nb_sprite * sprite = &_nb.ram.sprite_table[index];

    switch(type)
    {
        //Regular 8x8
        case NB_SPRITE_REGULAR:
            size.x  = _nb.cache.sprite_resolution.x;
            size.y  = _nb.cache.sprite_resolution.y;
        break;
        //Tall 8x16    
        case NB_SPRITE_WIDE:
            size.x = _nb.cache.sprite_resolution.x * 2 ;
            size.y = _nb.cache.sprite_resolution.y;
        break;
         //Wide 16x8
        case NB_SPRITE_TALL:
            size.x = _nb.cache.sprite_resolution.x;
            size.y = _nb.cache.sprite_resolution.y * 2 ;
        break;
    }


    nb_init_sprite(sprite, &_nb.gfx.shaders[NB_SPRITE_SHADER], offset, size);

    return  sprite;
}

void        nb_remove_sprite(nb_sprite * sprite)
{
    nb_destroy_sprite(sprite); //set sheet to null, no longer renderable    
}


