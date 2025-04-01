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
layout (location = 3) out vec4 fragColor3;

uniform sampler2D _MainTex;

struct Material 
{
	float roughness;
	float metalic;
};

uniform Material _Material;

void main() 
{
	vec3 normal = normalize(fs_surface.worldNormal);

	vec3 objectColor0 = texture(_MainTex, fs_surface.texcoord).rgb;
	vec3 position = vec3(fs_surface.worldPos.xyz);
	vec3 normals = vec3(normal.xyz);

	fragColor0 = vec4(objectColor0, 1.0);
	fragColor1 = vec4(position, 1.0);
	fragColor2 = vec4(normals, 1.0);
	fragColor3 = vec4(_Material.metalic, _Material.roughness, 0, 0);
}