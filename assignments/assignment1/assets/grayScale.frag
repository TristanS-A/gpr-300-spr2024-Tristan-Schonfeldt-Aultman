#version 450

out vec4 FragColor;

in vec2 vs_textcoords;

uniform sampler2D _MainTex;

void main() 
{
	vec3 albedoColor = texture(_MainTex, vs_textcoords).rgb;
	
	// naive
	//float average = (albedoColor.r + albedoColor.g + albedoColor.b) / 3.0f;
	
	//realistic
	float average = (albedoColor.r * 0.2126) + (albedoColor.g * 0.7152) + (albedoColor.b * 0.0722);
	FragColor = vec4(vec3(average), 1.0);
}