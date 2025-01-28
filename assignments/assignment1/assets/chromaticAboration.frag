#version 450

out vec4 FragColor;

in vec2 vs_textcoords;

uniform sampler2D _MainTex;

const vec3 offset = vec3(0.009, 0.006, -0.006);
const vec2 direction = vec2(1.0);

void main() 
{
	FragColor.r = texture(_MainTex, vs_textcoords + (direction * vec2(offset.r))).r;
	FragColor.g = texture(_MainTex, vs_textcoords + (direction * vec2(offset.g))).g;
	FragColor.b = texture(_MainTex, vs_textcoords + (direction * vec2(offset.b))).b;
}