#version 450

in vec3 out_Normal;
in Surface 
{
	vec3 worldPos;
	vec3 worldNormal;
	vec4 fragPosLightSpace;
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

struct Light 
{
	vec3 color;
	vec3 pos;
};

out vec4 FragColor;

uniform sampler2D _MainTex;
uniform sampler2D _NormalMap;
uniform sampler2D _ShadowMap;
uniform vec3 _EyePos;
uniform Material _Material;
uniform Light _Light;
uniform float _ShadowBias;
uniform int _PCFFactor;

uniform vec3 _CamPos;
//uniform vec3 _AmbientColor = vec3(0.3, 0.4, 0.46);

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

//Calculates Blinn Phong lighting
vec3 blinnPhong(vec3 normal, vec3 fragPos, vec3 lightDir)
{
	//Normalize inputs
	vec3 viewDir = normalize(_CamPos - fragPos);
	vec3 halfDir = normalize(_Light.pos + viewDir);

	//Dot products
	float nDotL = max(dot(normal, lightDir), 0.0);
	float nDotH = max(dot(normal, halfDir), 0.0);

	vec3 diffuse = vec3(nDotL * _Material.diffuseK);
	vec3 specular = vec3(pow(nDotH, _Material.shininess * 128.0) * _Material.specularK);

	return (diffuse + specular);
}

void main() 
{
	vec3 normal = normalize(fs_surface.worldNormal);
	vec3 lightDir = normalize(_Light.pos - fs_surface.worldPos);

	float shadow = shadowCalculations(fs_surface.fragPosLightSpace, lightDir, normal);
	
	vec3 lighting = blinnPhong(normal, fs_surface.worldPos, lightDir);

	vec3 objectColor = texture(_MainTex, fs_surface.texcoord).rgb;

	vec3 finalLighting = _Light.color * ((_Material.ambientK + (1.0 - shadow) * lighting) * objectColor);
	FragColor = vec4(finalLighting, 1.0);
}