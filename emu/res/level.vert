#version 130
//uniform usampler2D palette;
//uniform usampler2D atlas;

uniform ivec2   screen_resolution;
uniform ivec2   scroll;
uniform vec2    rect_size;
uniform vec2    rect_pos;
uniform int     screen_scale;

in vec2         in_vert;
in vec2         in_uv;

out vec2        uv;

void main()
{
    uv= in_uv;
    //map to NDC -1,1
//subtract scroll. moving rect to the right, looks as if level moves left
    uv += vec2(scroll)/rect_size;

    vec2 ndc_pos = (rect_size*in_vert+rect_pos )/screen_resolution*2.0-1.0;
    gl_Position =vec4(ndc_pos,0,1)*screen_scale  ;
}