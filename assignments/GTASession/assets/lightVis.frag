#version 450

layout (location = 4) out vec4 fragColor4;

uniform vec3 _Color;

void main() 
{
	fragColor4 = vec4(_Color, 1.0);
}