#version 450

out vec4 FragColor;

in vec2 vs_textcoords;

uniform sampler2D _MainTex;
uniform float _Gamma;
uniform float _Exposure;

void main() 
{
    vec3 hdrColor = texture(_MainTex, vs_textcoords).rgb;
  
    // exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdrColor * _Exposure);
    // gamma correction 
    mapped = pow(mapped, vec3(_Gamma));
  
    FragColor = vec4(mapped, 1.0);
}