#version 450

in vec3 out_Normal;
in Surface 
{
	vec3 worldPos;
	vec3 worldNormal;
	vec2 texcoord;
} vs_surface;

struct Material 
{
	float ambientK;
	float diffuseK;
	float specularK;
	float shininess;
};

out vec4 FragColor;

uniform sampler2D _MainTex;
uniform vec3 _EyePos;
uniform Material _Material;

//Light source
uniform vec3 _LightDir = vec3(0.0, 1.0, 0.0);
uniform vec3 _LightCol = vec3(1.0, 0.0, 1.0);
uniform vec3 _AmbientColor = vec3(0.3, 0.4, 0.46);

void main() 
{
	//Compute necessary vectors
	vec3 normal = normalize(vs_surface.worldNormal);
	vec3 toLight = _LightDir;
	vec3 toEye = normalize(_EyePos - vs_surface.worldPos);

	//Calculate diffuse lighting
	float diffuseFactor = max(dot(normal, toLight), 0.0);

	//Calculate specular lighting
	vec3 halfVec = normalize(toLight + toEye);
	float specularFactor = pow(max(dot(normal, halfVec), 0.0), _Material.shininess);

	vec3 lightColor = (diffuseFactor * _Material.diffuseK + specularFactor * _Material.specularK) * _LightCol;

	//Add ambient light
	lightColor += _AmbientColor * _Material.ambientK;

	//FragColor = vec4(out_Normal * 0.5f, 1.0);
	vec3 objectColor = texture(_MainTex, vs_surface.texcoord).rgb;
	FragColor = vec4(objectColor * lightColor, 1.0);
}