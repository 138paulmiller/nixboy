#version 130

uniform ivec2   screen_resolution;
uniform vec2    rect_size;
uniform vec2    rect_pos;
uniform int     screen_scale;

in vec2         in_vert;
in vec2         in_uv;

out vec2        uv;

void main()
{
    uv=in_uv;
    //map to NDC -1,1
    vec2 ndc_pos = (rect_size*in_vert+rect_pos)/screen_resolution*2.0-1.0;

    gl_Position =vec4(ndc_pos,0,1)*screen_scale;
}