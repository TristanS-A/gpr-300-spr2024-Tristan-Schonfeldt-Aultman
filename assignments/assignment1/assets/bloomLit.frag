#version 450

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 fragBright;

in vec3 out_Normal;
in Surface 
{
	vec3 worldPos;
	vec3 worldNormal;
	vec2 texcoord;
	mat3 TBN_Mat;
} fs_surface;

struct Material 
{
	float ambientK;
	float diffuseK;
	float specularK;
	float shininess;
};

uniform sampler2D _MainTex;
uniform sampler2D _NormalMap;
uniform vec3 _EyePos;
uniform Material _Material;

//Light source
uniform vec3 _LightDir;
uniform vec3 _LightCol;
uniform vec3 _AmbientColor;
uniform bool _Use_NormalMap;

const float brightnessThreshold = 1.0f;

void main() 
{
	//Compute necessary vectors
	vec3 toLight = _LightDir;
	vec3 toEye = normalize(_EyePos - fs_surface.worldPos);

	vec3 normal;
	//Branch for demonstration purposes
	if (_Use_NormalMap)
	{
		//Normal Map Calculations
		normal = texture(_NormalMap, fs_surface.texcoord).rgb;
		normal = normal * 2.0 - 1.0;
		normal = normalize(fs_surface.TBN_Mat * normal);
	}
	else 
	{
		normal = normalize(fs_surface.worldNormal);   //Old regular vertex normals
	}
	
	//Calculate diffuse lighting
	float diffuseFactor = max(dot(normal, toLight), 0.0);

	//Calculate specular lighting
	vec3 halfVec = normalize(toLight + toEye);
	float specularFactor = pow(max(dot(normal, halfVec), 0.0), _Material.shininess);

	//Combines specular and difusion light with light color
	vec3 lightColor = (diffuseFactor * _Material.diffuseK + specularFactor * _Material.specularK) * _LightCol;

	//Add ambient light
	lightColor += _AmbientColor * _Material.ambientK;

	//Calculate final light color
	//FragColor = vec4(normal * 0.5 + 0.5, 1.0);
	vec3 objectColor = texture(_MainTex, fs_surface.texcoord).rgb;
	fragColor = vec4(objectColor * lightColor, 1.0);

	//Check brightness
	float brightness = dot(fragColor.rgb, vec3(0.2126, 0.7151, 0.0722));
	if (brightness > brightnessThreshold)
	{
		fragBright = fragColor;
	}
	else 
	{
		fragBright = vec4(vec3(0.0), 1.0);
	}
}