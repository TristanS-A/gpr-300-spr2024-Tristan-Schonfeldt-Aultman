
#version 450

layout (location = 0) in vec3 v_In_Pos;
layout (location = 1) in vec3 v_In_Normal;
layout (location = 2) in vec3 aTangent;
layout (location = 3) in vec2 in_texcoord;

out vec3 out_Normal;

out Surface 
{
	vec3 worldPos;
	vec3 worldNormal;
	vec4 lightPos;
	vec2 texcoord;
	mat3 TBN_Mat;
} vs_surface;

uniform mat4 camera_viewProj;
uniform mat4 _LightViewProj;
uniform mat4 _Model;

uniform sampler2D _ShadowMap;

void main()
{
	vec3 newPos = v_In_Pos;
	vs_surface.lightPos = _LightViewProj * vec4(vec3(_Model * vec4(newPos, 1.0)), 1.0);

	//Perspective devide -> normalized device coords
	vec3 projectionCoords = vs_surface.lightPos.xyz / vs_surface.lightPos.w;

	//Map to 0-1
	projectionCoords = (projectionCoords * 0.5) + 0.5;

	float height = (texture(_ShadowMap, projectionCoords.xy).r - 1) * -1;
	newPos = v_In_Pos - vec3(0, height * 0.5, 0);

	vs_surface.worldPos = vec3(_Model * vec4(newPos, 1.0));
	vs_surface.worldNormal = transpose(inverse(mat3(_Model))) * v_In_Normal;

	vs_surface.texcoord = in_texcoord;
	gl_Position = camera_viewProj * _Model * vec4(newPos, 1.0);
}