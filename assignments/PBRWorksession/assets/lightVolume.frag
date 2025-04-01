#version 450

layout (location = 0) out vec4 fragColor0;

uniform sampler2D _Albedo;
uniform sampler2D _PositionTex;
uniform sampler2D _NormalTex;
uniform sampler2D _MaterialTex;

struct Light 
{
	vec3 color;
	vec3 pos;
	float radius;
};

uniform Light _Light;

struct Material 
{
	float ambientK;
	float diffuseK;
	float specularK;
	float shininess;
};

uniform vec3 _CamPos;

vec3 calculateLighting(vec3 lightDir, vec3 worldPos, vec3 normal, vec2 UV)
{
	//Normalize inputs
	vec3 viewDir = normalize(_CamPos - worldPos);
	vec3 halfDir = normalize(lightDir + viewDir);

	//Dot products
	float nDotL = max(dot(normal, lightDir), 0.0);
	float nDotH = max(dot(normal, halfDir), 0.0);

	vec4 materialInfo = texture(_MaterialTex, UV).rgba;

	vec3 diffuse = vec3(nDotL * materialInfo.g);
	vec3 specular = vec3(pow(nDotH, materialInfo.a) * materialInfo.b);

	return (diffuse + specular);
}

float calculateAttentuation(float dist, float radius)
{
	float i = clamp(1.0 - pow(dist/radius,4.0),0.0,1.0);
	return i * i;
}

void main() 
{
	vec2 UV = gl_FragCoord.xy / textureSize(_NormalTex, 0);
	vec3 normal = texture(_NormalTex, UV).rgb;
	vec3 worldPos = texture(_PositionTex, UV).rgb;
	vec3 albedo = texture(_Albedo, UV).rgb;

	vec3 toLightVec = _Light.pos - worldPos;
	vec3 normalizedLightDir = normalize(toLightVec);

	float attentuation = calculateAttentuation(length(toLightVec), _Light.radius);

	vec3 lightColor = calculateLighting(normalizedLightDir, worldPos, normal, UV) * _Light.color * attentuation;
	fragColor0 = vec4(lightColor * albedo, 1.0);
}