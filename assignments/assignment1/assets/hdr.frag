#version 450

out vec4 FragColor;

in vec2 vs_textcoords;

uniform sampler2D _MainTex;
uniform sampler2D _DepthTex;
uniform float _Near;
uniform float _Far;
uniform vec3 _FogColor;


void main() 
{
	const float gamma = 2.2;
    vec3 hdrColor = texture(_MainTex, vs_textcoords).rgb;
  
    // reinhard tone mapping
    vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));
  
    FragColor = vec4(mapped, 1.0);
}