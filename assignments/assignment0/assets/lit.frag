#version 450

in vec3 out_Normal;
in Surface 
{
	vec3 worldPos;
	vec3 worldNormal;
	vec2 texcoord;
} vs_surface;

out vec4 FragColor;

uniform sampler2D _MainTex;
uniform vec3 _EyePos;

//Light source
uniform vec3 _LightDir = vec3(0.0, -1.0, 0.0);
uniform vec3 _LightCol = vec3(1.0, 0.0, 1.0);

void main() 
{
	vec3 normal = normalize(vs_surface.worldNormal);
	vec3 toLight = _LightDir;
	vec3 toEye = normalize(_EyePos - vs_surface.worldPos);

	float diffuseFactor = max(dot(normal, toLight), 0.0);
	vec3 diffuseColor = _LightCol * diffuseFactor;

	vec3 halfVec = normalize(toLight + toEye);
	float specularFactor = pow(max(dot(normal, halfVec), 0.0), 128);

	vec3 lightColor = (diffuseColor + specularFactor) * _LightCol;

	//FragColor = vec4(out_Normal * 0.5f, 1.0);
	vec3 objectColor = texture(_MainTex, vs_surface.texcoord).rgb;
	FragColor = vec4(objectColor * lightColor, 1.0);
}