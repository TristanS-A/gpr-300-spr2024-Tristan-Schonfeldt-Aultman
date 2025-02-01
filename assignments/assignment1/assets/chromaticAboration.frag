#version 450

out vec4 FragColor;

in vec2 vs_textcoords;

uniform sampler2D _MainTex;

uniform vec3 _Offset;
uniform vec2 _Direction;

void main() 
{
	FragColor.r = texture(_MainTex, vs_textcoords + (_Direction * vec2(_Offset.r))).r;
	FragColor.g = texture(_MainTex, vs_textcoords + (_Direction * vec2(_Offset.g))).g;
	FragColor.b = texture(_MainTex, vs_textcoords + (_Direction * vec2(_Offset.b))).b;
}