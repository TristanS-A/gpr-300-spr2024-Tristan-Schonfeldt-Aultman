#version 450

out vec4 FragColor;

in vec2 vs_textcoords;

uniform sampler2D _Albedo;
uniform sampler2D _Lighting;
uniform sampler2D _MaterialInfo;

uniform vec3 _CamPos;

void main() 
{
   vec3 objectColor = texture(_Albedo, vs_textcoords).rgb;
   vec3 lightingColor = texture(_Lighting, vs_textcoords).rgb;
   vec3 materialInfo = texture(_MaterialInfo, vs_textcoords).rgb;
   
   vec3 finalLighting = objectColor * lightingColor;

   FragColor = vec4(finalLighting, 1.0);
}