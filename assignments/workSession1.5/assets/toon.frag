#version 450

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

out vec4 FragColor;

uniform sampler2D _MainTex;
uniform sampler2D _ZaToon;
uniform sampler2D _NormalMap;
uniform vec3 _EyePos;
uniform Material _Material;
uniform vec3 _Highlight;
uniform vec3 _Shadow;

//Light source
uniform vec3 _LightDir = vec3(0.0, 1.0, 0.0);
uniform vec3 _LightCol = vec3(1.0, 1.0, 1.0);
uniform vec3 _AmbientColor = vec3(0.3, 0.4, 0.46);
uniform bool _Use_NormalMap;

vec3 toonLighting(vec3 normal, vec3 lightDir)
{
	float diff = (dot(normal, lightDir) + 1.0) * 0.5;
	vec3 lightColor = vec3(1.0) * diff;
	float step = texture(_ZaToon, vec2(diff)).r;
	lightColor = mix(_Shadow, _Highlight, step);
	return lightColor * step;
}

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

	//Combines specular and difusion light with light color
	vec3 lightColor = toonLighting(normal, _LightDir);

	//Calculate final light color
	//FragColor = vec4(normal * 0.5 + 0.5, 1.0);
	vec3 objectColor = texture(_MainTex, fs_surface.texcoord).rgb;
	FragColor = vec4(objectColor * lightColor, 1.0);
}