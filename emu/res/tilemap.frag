#version 130

uniform usampler1D 	palette;
uniform usampler2D 	atlas;
uniform usampler2D 	tilemap;

uniform ivec2   	screen_resolution;
uniform ivec2   	atlas_resolution;
uniform ivec2     	tilemap_resolution;
uniform ivec2   	scroll;
uniform ivec2    	tile_size;
uniform vec2    	rect_size;
uniform vec2    	rect_pos;
uniform int   		screen_scale;
uniform int    		color_depth;
uniform int    		palette_size;
//
in  vec2	 		uv;
out vec4 	 		out_color;

//#define CHECK_TILE_INDEX 

uint get_tile_index()
{
	vec2 res 			= vec2(tilemap_resolution);
	vec2 tilemap_uv 	= (uv * res ) / res;
	uint tile_index  	= texture(tilemap, tilemap_uv).r;

	return tile_index;
}

vec2 compute_atlas_uv(uint tile_index)
{
	
	vec2 tiles_per_tilemap = vec2(tilemap_resolution);
	vec2 tiles_per_atlas = vec2(atlas_resolution) / vec2(tile_size);

	int atlas_width  = int(tiles_per_atlas.x);
	float tile_index_f = float(tile_index);
	
	//calc offset with uv in range of atlas dimension
	float u = int(tile_index) % int(tiles_per_atlas.x);
	float v = int(tile_index) / int(tiles_per_atlas.y);
	
	vec2 origin_uv = vec2( u, v ) / tiles_per_atlas ;
	
	
	float u_offset = fract(uv.x * float(tiles_per_tilemap.x)) / tiles_per_atlas.x;
	float v_offset = fract(uv.y * float(tiles_per_tilemap.y)) / tiles_per_atlas.y;
	vec2 uv_offset 	= vec2(u_offset, v_offset); 
	
	vec2 atlas_uv = (origin_uv + uv_offset ) ;
	return atlas_uv ; 
}

vec4 sample_palette(uint color_index)
{
	float color_sample = float(color_index)/float(palette_size);
	return 		(texture(palette, color_sample  ))/float(color_depth);

}

void main()
{
	uint tile_index 	=  get_tile_index();
	vec2 atlas_uv 	= compute_atlas_uv(tile_index);
//	out_color = vec4(atlas_uv, 0,1);
//	return ;

	uint color_index = (texture(atlas, atlas_uv).r);
	out_color = sample_palette(color_index);

}
