#version 130

uniform usampler1D 	palette;
uniform usampler2D 	atlas;
uniform ivec2   	screen_resolution;
uniform ivec2   	atlas_resolution;
uniform ivec2   	scroll;

uniform int   		screen_scale;
uniform int    		palette_size;
uniform int    		color_depth;

//
in  vec2	 		uv;
out vec4 	 		out_color;
	
void main()
{
	float color_index  = (texture(atlas, uv).r)/float(palette_size) ;

	out_color = vec4(uv,1,0);//(texture(palette, color_index))/float(color_depth);
}//
