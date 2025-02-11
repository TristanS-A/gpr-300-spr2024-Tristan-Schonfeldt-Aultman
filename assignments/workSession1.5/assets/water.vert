#version 450

layout (location = 0) in vec3 v_In_Pos;
layout (location = 1) in vec3 v_In_Normal;
layout (location = 2) in vec3 aTangent;
layout (location = 3) in vec2 in_texcoord;

out vec3 out_Normal;
out vec3 toCam;

out Surface 
{
	vec2 texcoord;
} vs_surface;

uniform mat4 camera_viewProj;
uniform mat4 _Model;
uniform vec3 _CamPos;
uniform float _Time;
uniform float _Strength;

float calculateSurface(float x, float z)
{
	float y = 0.0;
	y += sin(x + _Time);
	y -= sin(z + _Time) + sin(x * 1.8);
	return y;
}

void main()
{
	vs_surface.texcoord = in_texcoord;
	vec4 worldPos = _Model * vec4(v_In_Pos + calculateSurface(v_In_Pos.x, v_In_Pos.z) * _Strength, 1.0);
	toCam = _CamPos - worldPos.xyz;
	gl_Position = camera_viewProj * worldPos;
}