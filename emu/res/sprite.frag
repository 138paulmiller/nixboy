#version 130

uniform usampler1D palette;
uniform usampler2D atlas;
uniform ivec2   screen_resolution;
uniform ivec2   atlas_resolution;
uniform vec2    size;
uniform ivec2   offset;
uniform vec2    pos;
uniform int   	scale;
uniform int    	palette_size;
uniform int    	color_depth;

in  vec2	 uv;
out vec4 	 out_color;


//get color index from atlas. grab color from palette using index  
vec4 get_color(vec2 atlas_uv)
{
	//render entire atlas
	uint color_index  = uint(texture(atlas, atlas_uv).r);
	return vec4 ( color_index/ float(palette_size) ) ;
	//Get Color from palette
	float coord  = (float(color_index)/ float(palette_size));
	vec4 sample_color = texture(palette, coord)/ float(color_depth);
	return  sample_color   ;
}

#define DEBUG_ATLAS 0

void main()
{
#if DEBUG_ATLAS
	out_color = texture(atlas, uv)/vec4(palette_size,1,1,1);
	return;	
#endif
	vec2 pixel = mod(uv, scale); 
	
	vec2 sprite_per_atlas = atlas_resolution / size;

	vec2 sprite_uv =  pixel*size+offset*sprite_per_atlas;

	//Get Color index
	sprite_uv =sprite_uv/atlas_resolution;

	out_color = get_color(sprite_uv);
}

