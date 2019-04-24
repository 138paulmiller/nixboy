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
	float color_index  = (texture(atlas, uv).r)/float(palette_size) ;

	out_color = (texture(palette, color_index))/float(color_depth);
}//
