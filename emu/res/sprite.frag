#version 130

uniform usampler1D palette;
uniform usampler2D atlas;
uniform ivec2   resolution;
uniform ivec2   offset;
uniform vec2    size;
uniform vec2    pos;
uniform int   	scale;
uniform int    	palette_size;
uniform int    	sprite_atlas_size;
uniform int    	color_depth;

in vec2 uv;
out vec4 out_color;
void main()
{
	
	//uint color_index  = uint(textureOffset(atlas, uv, vec2(offset)/(size) ).r);
	//scale uv by texture size

	vec2 pixel = mod(uv, scale); 
	float atlas_width = sprite_atlas_size;
	
	vec2 atlas_dimen = vec2(atlas_width,atlas_width);
	vec2 sprite_per_atlas = atlas_dimen / size;

	vec2 sprite_uv =  pixel*size+offset;

	//Get Color index
	sprite_uv =sprite_uv/atlas_dimen;
	uint color_index  = uint(texture(atlas, sprite_uv).r);


	//Get Color from palette
	float coord  = float(color_index)/ float(palette_size);
	uvec4 sample_color = texture(palette, coord);
	out_color =  vec4( sample_color / float(color_depth) ) ;

}