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

uniform sampler2D _MainTex;
uniform sampler2D _NormalMap;
uniform vec3 _EyePos;

//Light source
//uniform vec3 _LightDir = vec3(0.0, 1.0, 0.0);
//uniform vec3 _LightCol = vec3(1.0, 1.0, 1.0);

void main() 
{
	//Compute necessary vectors
	//vec3 toLight = _LightDir;
	//vec3 toEye = normalize(_EyePos - fs_surface.worldPos);

	//vec3 normal = normalize(fs_surface.worldNormal);   //Old regular vertex normals
	
	//Calculate diffuse lighting
	//float diffuseFactor = max(dot(normal, toLight), 0.0);

	//Calculate specular lighting
	//vec3 halfVec = normalize(toLight + toEye);
	//float specularFactor = pow(max(dot(normal, halfVec), 0.0), _Material.shininess);

	//Combines specular and difusion light with light color
	//vec3 lightColor = (diffuseFactor * _Material.diffuseK + specularFactor * _Material.specularK) * _LightCol;

	//Add ambient light
	//lightColor += _AmbientColor * _Material.ambientK;

	//Calculate final light color
	//FragColor = vec4(normal * 0.5 + 0.5, 1.0);
	vec3 objectColor0 = texture(_MainTex, fs_surface.texcoord).rgb;
	vec3 objectColor1 = vec3(fs_surface.worldPos.xyz);;

	//FragColor = vec4(objectColor * lightColor, 1.0);
	fragColor0 = vec4(objectColor0, 1.0);
	fragColor1 = vec4(objectColor1, 1.0);
}