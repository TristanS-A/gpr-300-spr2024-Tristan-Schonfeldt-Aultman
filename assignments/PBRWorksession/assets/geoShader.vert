#version 450

layout (location = 0) in vec3 v_In_Pos;
layout (location = 1) in vec2 in_texcoord;

out vec2 vs_textcoords;

void main()
{
	vs_textcoords = in_texcoord;
	gl_Position = vec4(v_In_Pos.xy, 0.0, 1.0);
}