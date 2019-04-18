#version 130

uniform usampler1D 	palette;
uniform usampler2D 	atlas;
uniform ivec2   	screen_resolution;
uniform ivec2   	atlas_resolution;
uniform vec2    	size;
uniform ivec2   	offset;
uniform vec2    	pos;
uniform int   		screen_scale;
uniform int    		palette_size;
uniform int    		color_depth;

//
in  vec2	 		uv;
out vec4 	 		out_color;
	
void main()
{

	vec2 min_offset=vec2(0,0);
	vec2 max_offset=vec2(atlas_resolution-size);
	vec2 clamped_offset = clamp(offset, min_offset, max_offset);

	vec2 sprite_uv =  (uv*size+clamped_offset)/atlas_resolution;

	float color_index  = (texture(atlas, sprite_uv).r)/float(palette_size) ;

	out_color = (texture(palette, color_index))/float(color_depth);
}//

