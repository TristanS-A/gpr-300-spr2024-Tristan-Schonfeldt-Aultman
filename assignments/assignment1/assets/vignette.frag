#version 450

out vec4 FragColor;

in vec2 vs_textcoords;

uniform sampler2D _MainTex;
uniform float _Radius;
uniform float _Smooth;

float circle(vec2 pos, float radius)
{
    return length(pos) - radius;
}

void main() 
{
    float cuttoff = circle(vs_textcoords - 0.5, _Radius);
    cuttoff = 1.0 - smoothstep(0.0, _Smooth, cuttoff);

    vec3 color = texture(_MainTex, vs_textcoords).rgb;
    color = mix(vec3(0.0, 0.5, 0.0), color, cuttoff);

    FragColor = vec4(color, 1.0);
}