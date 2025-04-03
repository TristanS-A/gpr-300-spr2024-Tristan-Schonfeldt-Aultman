#version 450

layout (location = 0) out vec4 fragColor0;

uniform sampler2D _Albedo;
uniform sampler2D _PositionTex;
uniform sampler2D _NormalTex;
uniform sampler2D _MaterialTex;
uniform sampler2D _ShadowMap;

struct Light 
{
	vec3 color;
	vec3 pos;
	float radius;
};

uniform Light _Light;
uniform Light _ShadowLight;

struct Material 
{
	float ambientK;
	float diffuseK;
	float specularK;
	float shininess;
};

uniform vec3 _CamPos;
uniform mat4 _LightViewProj;

uniform int _PCFFactor = 1;
uniform float _ShadowBias = 0.01;

vec3 calculateLighting(vec3 lightDir, vec3 worldPos, vec3 normal, vec2 UV, vec4 matInfo)
{
	//Normalize inputs
	vec3 viewDir = normalize(_CamPos - worldPos);
	vec3 halfDir = normalize(lightDir + viewDir);

	//Dot products
	float nDotL = max(dot(normal, lightDir), 0.0);
	float nDotH = max(dot(normal, halfDir), 0.0);

	vec3 diffuse = vec3(nDotL * matInfo.g);
	vec3 specular = vec3(pow(nDotH, matInfo.a) * matInfo.b);

	return (diffuse + specular) * _Light.color;
}

//Calculates shadows using shadow map
float shadowCalculations(vec4 fragPosLightSpace, vec3 lightDir, vec3 normal)
{
	//Perspective devide -> normalized device coords
	vec3 projectionCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

	//Map to 0-1
	projectionCoords = (projectionCoords * 0.5) + 0.5;

	////Way to get rid of plane out of camera view issues (over sampling) for shadows
	if (projectionCoords.z  <= 0.0 || projectionCoords.z > 1.0)
	{
		return 0.0;
	}

	float currentDepth = projectionCoords.z;

	float bias = max(0.01 * (1.0 - dot(normal, lightDir)), _ShadowBias);  //Scales bias to light angle

	//Branch for ImGui to show PCF vs no PCF
	if (_PCFFactor != 0)
	{
		//Calculates PCF
		float shadow = 0.0;
		vec2 texelSize = 1.0 / textureSize(_ShadowMap, 0);
		int num = 0;
		for (int x = -_PCFFactor; x < _PCFFactor; ++x)
		{
			for (int y = -_PCFFactor; y < _PCFFactor; ++y)
			{
				float pcfDepth = texture(_ShadowMap, projectionCoords.xy + vec2(x, y) * texelSize).r;
				shadow += ((currentDepth - bias) > pcfDepth) ? 1.0 : 0.0;
				num++;
			}
		}

		return shadow / (_PCFFactor * 2 * _PCFFactor * 2);
	}
	else 
	{
		float closestDepth = texture(_ShadowMap, projectionCoords.xy).r;
		float currentDepth = projectionCoords.z;

		float shadow = ((currentDepth - bias) > closestDepth) ? 1.0 : 0.0;
		return shadow;
	}
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
	vec4 materialInfo = texture(_MaterialTex, UV).rgba;

	vec3 toLightVec = _Light.pos - worldPos;
	vec3 normalizedLightDir = normalize(toLightVec);

	float attentuation = calculateAttentuation(length(toLightVec), _Light.radius);

	vec3 shadowLightDir = normalize(_ShadowLight.pos - worldPos);
	float shadow = shadowCalculations(_LightViewProj * vec4(worldPos, 1.0), shadowLightDir, normal);

	vec3 lighting = calculateLighting(normalizedLightDir, worldPos, normal, UV, materialInfo) + albedo * materialInfo.r;

	vec3 lightColor = lighting * attentuation * (1.0 - shadow);
	fragColor0 = vec4(lightColor * albedo, 1.0);
}