#version 450

layout (location = 3) out vec4 fragColor3;

in vec2 vs_textcoords;

uniform sampler2D _PositionTex;
uniform sampler2D _NormalTex;

uniform vec3 _CamPos;

struct Light 
{
	vec3 color;
	vec3 pos;
};

uniform int lightsLength = 100;
uniform Light[100] _Lights;

struct Material 
{
	float ambientK;
	float diffuseK;
	float specularK;
	float shininess;
};

uniform Material _Material;

//Calculates Blinn Phong lighting
vec3 blinnPhong(vec3 normal, vec3 fragPos, vec3 lightDir)
{
	//Normalize inputs
	vec3 viewDir = normalize(_CamPos - fragPos);
	vec3 halfDir = normalize(lightDir + viewDir);

	//Dot products
	float nDotL = max(dot(normal, lightDir), 0.0);
	float nDotH = max(dot(normal, halfDir), 0.0);

	vec3 diffuse = vec3(nDotL * _Material.diffuseK);
	vec3 specular = vec3(pow(nDotH, _Material.shininess) * _Material.specularK);

	return (diffuse + specular);
}

void main() 
{
   vec3 positionColor = texture(_PositionTex, vs_textcoords).xyz;
   vec3 normalColor = texture(_NormalTex, vs_textcoords).xyz;

   vec3 finalLighting = vec3(0.0);

   for (int i = 0; i < lightsLength; i++)
   {
		vec3 lightDir = normalize(_Lights[i].pos - positionColor);

		vec3 lighting = blinnPhong(normalColor, positionColor, lightDir);
		finalLighting += _Lights[i].color * ((_Material.ambientK + lighting));
   }

   fragColor3 = vec4(1.0, 0.0, 1.0, 1.0);
}