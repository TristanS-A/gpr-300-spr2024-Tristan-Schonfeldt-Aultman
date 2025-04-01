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
	float roughness;
	float metalic;
};

uniform vec3 _CamPos;

uniform float _Absalom;

const float PI = 3.1415926;

//Caching dots
float NdotV = 0.0;
float NdotL = 0.0;
float NdotH = 0.0;

vec3 F() { return vec3(1.0); };
vec3 G() { return vec3(1.0); };
float D(float roughness) 
{
	//Alpha = pow(roughness, 2.0);
	float alpha2 = pow(roughness, 4.0);
	float denominator = PI * pow(pow(NdotH, 2.0) * (alpha2 - 1.0) + 1.0, 2.0);

	return alpha2 / denominator;
};

vec3 cookTolorrance(vec3 fresnel, float roughness)
{
	vec3 numerator = D(roughness) * fresnel * G();
	float denominator = 4.0 * NdotV * NdotL;

	return numerator / denominator;
}

float calculateAttentuation(float dist, float radius)
{
	float i = clamp(1.0 - pow(dist/radius,4.0),0.0,1.0);
	return i * i;
}

vec3 fresnelSchlick(vec3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - NdotL, 5.0);
}

vec3 BDRF(vec3 toLight, vec3 worldPos, vec3 albedo, vec3 normal, vec2 UV)
{
	vec4 materialInfo = texture(_MaterialTex, UV).rgba;

	vec3 lambert = albedo / PI;

	vec3 F0 = lambert;

	vec3 kS = fresnelSchlick(F0);
	vec3 kD = (1 - kS) * (1.0 - materialInfo.r);

	vec3 diffuse = kD * lambert;
	vec3 specular = kS * cookTolorrance(kS, materialInfo.g);

	float attentuation = calculateAttentuation(length(toLight), _Light.radius);

	return (diffuse + specular) * attentuation * NdotL;
}

vec3 CalculateOutgoingLight(vec3 fragPos, vec3 albedo, vec3 normal, vec2 UV)
{	
	vec3 emitted = vec3(0.0);
	vec3 radiance = vec3(0.0);

	vec3 incomingColor = _Light.color;
	vec3 incomingToLight = _Light.pos - fragPos;

	vec3 lightDir = normalize(incomingToLight);
	vec3 viewDir = normalize(_CamPos - fragPos);
	vec3 halfDir = normalize(lightDir + viewDir);

	NdotL = max(dot(lightDir, normal), _Absalom);
	NdotH = max(dot(normal, halfDir), _Absalom);
	NdotV = max(dot(viewDir, normal), _Absalom);

	vec3 bdrf = BDRF(incomingToLight, fragPos, albedo, normal, UV);

	radiance += bdrf * incomingColor;

	return radiance + emitted;
}

void main() 
{
	vec2 UV = gl_FragCoord.xy / textureSize(_NormalTex, 0);
	vec3 normal = texture(_NormalTex, UV).rgb;
	vec3 worldPos = texture(_PositionTex, UV).rgb;
	vec3 albedo = texture(_Albedo, UV).rgb;

	vec3 outgoingLight = CalculateOutgoingLight(worldPos, albedo, normal, UV);

	fragColor0 = vec4(outgoingLight, 1.0);
}