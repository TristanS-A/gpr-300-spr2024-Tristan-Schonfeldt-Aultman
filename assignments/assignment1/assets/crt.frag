#version 450

out vec4 FragColor;

in vec2 vs_textcoords;

uniform sampler2D _MainTex;
uniform float _Time;
uniform float _ScrollSpeed;
uniform float _ScanlineHeight;
uniform float _ScanlineIntensity;
uniform float _GrainIntensity;
uniform float _AffectIntensity;

void main() 
{
    float scanline = sin((vs_textcoords.y - _Time * _ScrollSpeed) * 1.0/_ScanlineHeight);
    vec3 crtColor = vec3(0.0, 0.0, 1.0) * scanline * _ScanlineIntensity;

    vec2 grainSeed = vec2(12.9898, 78.233) + _Time * 0.1;
    float grain = fract(sin(dot(vs_textcoords, grainSeed)) * 43758.5453);
    crtColor += grain * _GrainIntensity;

    vec3 originalColor = texture(_MainTex, vs_textcoords).rgb;
    vec3 finalColor = mix(originalColor, crtColor, _AffectIntensity);

    FragColor = vec4(finalColor, 1.0);
}