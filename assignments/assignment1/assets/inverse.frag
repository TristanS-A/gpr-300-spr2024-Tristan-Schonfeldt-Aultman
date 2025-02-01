#version 450

out vec4 FragColor;

in vec2 vs_textcoords;

uniform sampler2D _MainTex;
uniform float _InverseSlider;

void main() 
{
	vec3 albedoColor = texture(_MainTex, vs_textcoords).rgb;

	albedoColor = mix(albedoColor, 1.0 - albedoColor, _InverseSlider);

	FragColor = vec4(albedoColor, 1.0);
}