#version 450

out vec4 FragColor;

in vec2 vs_textcoords;

uniform sampler2D _MainTex;
uniform vec2 _PSXResolution;
uniform vec2 _NewResolution;

void main() 
{
    vec2 scale = _PSXResolution / _NewResolution;
    vec3 finalColor = texture(_MainTex, vs_textcoords).rgb;

    FragColor = vec4(finalColor, 1.0);
}