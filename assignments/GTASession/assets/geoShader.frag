#version 450

out vec4 FragColor;

in vec2 vs_textcoords;

uniform sampler2D _Albedo;
uniform sampler2D _PositionTex;
uniform sampler2D _NormalTex;

void main() 
{
   vec3 color = texture(_Albedo, vs_textcoords).rgb;

   FragColor = vec4(color, 1.0);
}