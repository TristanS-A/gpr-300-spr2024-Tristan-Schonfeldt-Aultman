#version 450

out vec4 FragColor;

in vec2 vs_textcoords;

uniform sampler2D _SceneColor;
uniform sampler2D _BloomBlurr;
uniform float _Gamma;
uniform float _Exposure;

void main() 
{
    vec3 hdrColor = texture(_SceneColor, vs_textcoords).rgb;      
    vec3 bloomColor = texture(_BloomBlurr, vs_textcoords).rgb;
    hdrColor += bloomColor; // additive blending

    // tone mapping
    vec3 result = vec3(1.0) - exp(-hdrColor * _Exposure);

    // also gamma correct while we're at it       
    result = pow(result, vec3(_Gamma));
    FragColor = vec4(result, 1.0);
}