#version 450

out vec4 FragColor;

in vec2 vs_textcoords;

uniform sampler2D _MainTex;

void main() 
{
	const float gamma = 2.2;
    vec3 hdrColor = pow(texture(_MainTex, vs_textcoords).rgb, vec3(gamma));
  
    // reinhard tone mapping
    vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));
  
    FragColor = vec4(mapped, 1.0);
}