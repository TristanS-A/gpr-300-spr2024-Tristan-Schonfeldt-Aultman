#version 450

out vec4 FragColor;

in vec2 vs_textcoords;

uniform sampler2D _MainTex;
uniform vec3 _RealisticScale;
uniform float _AvarageScale;

void main() 
{
	vec3 albedoColor = texture(_MainTex, vs_textcoords).rgb;
	
	// naive
	//float average = (albedoColor.r + albedoColor.g + albedoColor.b) / 3.0f;
	
	//realistic
	float average = ((albedoColor.r * _RealisticScale.x) + (albedoColor.g * _RealisticScale.y) + (albedoColor.b * _RealisticScale.z)) / _AvarageScale;
	FragColor = vec4(vec3(average), 1.0);
}