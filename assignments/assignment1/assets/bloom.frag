#version 450

out vec4 FragColor;

in vec2 vs_textcoords;

uniform sampler2D _SceneColor;
uniform sampler2D _BloomBlurr;
float exposure = 1.0f;

void main() 
{
	const float gamma = 2.2;
    vec3 hdrColor = texture(_SceneColor, vs_textcoords).rgb;
    vec3 blurrColor = texture(_BloomBlurr, vs_textcoords).rgb;
    hdrColor += blurrColor;
  
    // Tone mapping
     vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
    // gamma correction 
   result = pow(result, vec3(1.0 / gamma));
  
    FragColor = vec4(result, 1.0);
}