#version 130

uniform usampler1D palette;
uniform usampler2D atlas;
uniform ivec2   screen_resolution;
uniform ivec2   atlas_resolution;
uniform ivec2   offset;
uniform vec2    size;
uniform vec2    pos;
uniform int   	scale;
uniform int    	palette_size;
uniform int    	color_depth;

in vec2 uv;
out vec4 out_color;
void main()
{
	
	vec2 pixel = mod(uv, scale); 
	
	vec2 sprite_per_atlas = atlas_resolution *scale ;

	//offset as index
	//vec2 sprite_uv =  pixel*size+offset*sprite_per_atlas;
	//vec2 sprite_uv =  clamp((pixel+vec2(offset) )*size,vec2(0,0),atlas_dimen) ;


	vec2 sprite_uv =  pixel*size  +offset/size  ;
	

	//Get Color index
	sprite_uv =sprite_uv/atlas_resolution;
	uint color_index  = uint(texture(atlas, sprite_uv).r);


	//Get Color from palette
	float coord  = (float(color_index)/ float(palette_size));
	uvec4 sample_color = texture(palette, coord);
	out_color =  vec4( sample_color / float(color_depth) ) ;

}

