#version 450

out vec4 FragColor;

in vec2 vs_textcoords;

uniform sampler2D _MainTex;
uniform float _Strength = 2.0;

const float offset = 1.0 / 300.0;
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
	1.0, 1.0, 1.0,
	1.0, -8.0, 1.0,
	1.0, 1.0, 1.0
);

void main() 
{
	vec3 col = vec3(0.0);

	for (int i = 0; i < 9; i++)
	{
		
		vec3 local = texture(_MainTex, vs_textcoords + offsets[i]).rgb;
		col += local * (kernal[i] / _Strength);
	}

	FragColor = vec4(vec3(col), 1.0);
}