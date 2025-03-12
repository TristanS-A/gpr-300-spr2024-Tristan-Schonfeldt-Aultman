#version 450

out vec4 FragColor;

in vec2 vs_textcoords;

uniform sampler2D _Albedo;
uniform sampler2D _LightingTex;
uniform sampler2D _Lights;


void main() 
{
   vec3 objectColor = texture(_Albedo, vs_textcoords).rgb;
   vec3 lightingColor = texture(_LightingTex, vs_textcoords).rgb;
   vec4 lights = texture(_Lights, vs_textcoords).rgba;
   
   vec3 finalLighting = lightingColor * objectColor;
   finalLighting = mix(finalLighting, lights.rgb, lights.a);

   FragColor = vec4(finalLighting, 1.0);
}