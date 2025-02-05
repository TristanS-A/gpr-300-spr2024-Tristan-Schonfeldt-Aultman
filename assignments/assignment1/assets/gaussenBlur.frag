#version 450

out vec4 FragColor;

in vec2 vs_textcoords;

uniform sampler2D _MainTex;
uniform float _InvStrength;

const float offset = 1.0 / _InvStrength;
const vec2 offsets[9] = vec2[]
(
	vec2(-offset, offset), //Top-left
	vec2(0.0, offset), //Top-center
	vec2(offset, offset), //Top-right

	vec2(-offset, 0.0), //Middle-left
	vec2(0.0, 0.0), //Middle-center
	vec2(offset, 0.0), //Middle-right

	vec2(-offset, -offset), //Bottom-left
	vec2(0.0, -offset), //Bottom-center
	vec2(offset, -offset) //Bottom-right

);

const float kernal[9] = float[]
(
	1.0, 2.0, 1.0,
	2.0, 4.0, 2.0,
	1.0, 2.0, 1.0
);

const float strength = 16;

void main() 
{
	vec3 average = vec3(0.0);

	float kernalSum = 0;
	for (int i = 0; i < 9; i++)
	{
		kernalSum += kernal[i];
	}

	for (int i = 0; i < 9; i++)
	{
		vec3 local = texture(_MainTex, vs_textcoords + offsets[i]).rgb;
		average += local * (kernal[i] / strength);
	}

	vec3 albedoColor = texture(_MainTex, vs_textcoords).rgb;
	FragColor = vec4(vec3(average), 1.0);
}