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


void main()
{
	vec2 min_scroll 	= vec2(0,0);
	vec2 max_scroll 	= vec2(rect_size);
	vec2 scaled_scroll =  scroll;

	vec2 tilemap_uv 		=  (uv * tilemap_resolution +scaled_scroll) / tilemap_resolution;
	
	float tile_index  	= (texture(tilemap, tilemap_uv).r);
	//map index to the atlas_uv_offset
	float atlas_width  = atlas_resolution.x;
	float mod_index = tile_index - (atlas_width * floor(tile_index/atlas_width));
	vec2 atlas_uv_offset = vec2(  mod_index , tile_index / atlas_width) ;

	vec2 tiles_per_atlas = atlas_resolution / tile_size ; 
	
	vec2 atlas_uv 		= atlas_uv_offset + (uv * atlas_resolution +scaled_scroll) / atlas_resolution;
	atlas_uv = atlas_uv/ tiles_per_atlas;
	float tilemap_sample = (texture(atlas, atlas_uv).r)/float(palette_size);
	out_color = vec4(atlas_uv,0,1);

out_color = (texture(palette, tilemap_sample))/float(color_depth);


/*
	vec2 min_scroll 	= vec2(0,0);
	vec2 max_scroll 	= vec2(atlas_resolution-rect_size);
	vec2 clamped_scroll = clamp(scroll, min_scroll, max_scroll);
	vec2 atlas_uv 		=  ( uv * rect_size + clamped_scroll ) / atlas_resolution;
	vec2 tile_per_edge = (atlas_resolution / rect_size);
	float tile_count    = tile_per_edge.x * tile_per_edge.y;

	float tile_index  	= (texture(atlas, atlas_uv).r)/float(tile_count);
	
	float tilemap_sample = (texture(tilemap, atlas_uv).r)/float(palette_size);
	out_color = (texture(palette, tile_index))/float(color_depth);
	out_color.r = tilemap_sample;
*/

}//
