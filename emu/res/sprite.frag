#version 130

uniform usampler1D palette;
uniform usampler2D atlas;
uniform ivec2   resolution;
uniform ivec2   offset;
uniform vec2    size;
uniform vec2    pos;
uniform int    	palette_size;
uniform int    	color_depth;

in vec2 uv;
out vec4 out_color;
void main()
{
	
//uint color_index  = uint(textureOffset(atlas, uv, offset).r);
	//scale uv by texture size
	vec2 sprite_uv = uv+ vec2(offset)/size;
	sprite_uv = sprite_uv/size;
	uint color_index  = uint(texture(atlas, sprite_uv).r);

	float coord  = float(color_index)/ float(palette_size);
	//normalize indices to float 
	uvec4 sample_color = texture(palette, coord);
	out_color =  vec4( sample_color / float(color_depth) ) ;
}
