#version 450

in vec3 out_Normal;
in Surface 
{
	vec3 worldPos;
	vec3 worldNormal;
	vec2 texcoord;
} fs_surface;

layout (location = 0) out vec4 fragColor0;
layout (location = 1) out vec4 fragColor1;
layout (location = 2) out vec4 fragColor2;

uniform sampler2D _MainTex;
//uniform sampler2D _NormalMap;
//uniform vec3 _EyePos;

//Light source
//uniform vec3 _LightDir = vec3(0.0, 1.0, 0.0);
//uniform vec3 _LightCol = vec3(1.0, 1.0, 1.0);

void main() 
{
	vec3 normal = normalize(fs_surface.worldNormal);

	vec3 objectColor0 = texture(_MainTex, fs_surface.texcoord).rgb;
	vec3 objectColor1 = vec3(fs_surface.worldPos.xyz);
	vec3 objectColor2 = vec3(normal.xyz);

	fragColor0 = vec4(objectColor0, 1.0);
	fragColor1 = vec4(objectColor1, 1.0);
	fragColor2 = vec4(objectColor2, 1.0);
}