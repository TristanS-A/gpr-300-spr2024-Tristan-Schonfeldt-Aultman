#version 450

layout (location = 0) in vec3 v_In_Pos;

out vec3 out_Normal;

uniform mat4 _LightViewProj;
uniform mat4 _Model;

void main()
{
	gl_Position = _LightViewProj * _Model * vec4(v_In_Pos, 1.0);
}