#version 450

layout (location = 0) in vec3 v_In_Pos;

uniform mat4 camera_viewProj;
uniform mat4 _Model;

void main()
{
	gl_Position = camera_viewProj * _Model * vec4(v_In_Pos, 1.0);
}