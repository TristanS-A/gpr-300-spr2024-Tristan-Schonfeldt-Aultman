#version 450

out vec4 FragColor;

in vec2 vs_textcoords;

uniform sampler2D _MainTex;
uniform sampler2D _DepthTex;
uniform float _Near;
uniform float _Far;
uniform vec3 _FogColor;

float linearizeDepth(float depth)
{
	float z = depth * 2.0 - 1.0;
	return ((2.0 * _Near * _Far) / (_Far + _Near - z * (_Far - _Near)));
}

float logisticDepth(float depth, float steepness, float offset) 
{
	float linearDepth = linearizeDepth(depth);
	return (1 / (1 + exp(-steepness * (linearDepth - offset))));
}

void main() 
{
	float depth = logisticDepth(texture(_DepthTex, vs_textcoords).r, 0.8, 10.0);

	vec3 color = mix(texture(_MainTex, vs_textcoords).rgb, _FogColor, depth);
	FragColor = vec4(color, 1.0);
}