#version 450

out vec4 FragColor;

in vec2 vs_textcoords;

uniform sampler2D _MainTex;

void main() 
{
	vec3 albedoColor = 1.0 - texture(_MainTex, vs_textcoords).rgb;
	FragColor = vec4(albedoColor, 1.0);
}