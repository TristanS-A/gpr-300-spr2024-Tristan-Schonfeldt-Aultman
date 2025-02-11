#version 450

in vec3 out_Normal;
in vec3 toCam;

in Surface 
{
	vec2 texcoord;
} fs_surface;

out vec4 FragColor;

uniform sampler2D _MainTex;
uniform vec3 _WaterColor;
uniform float _Tiling;
uniform float _Time;
uniform float _Blend1;
uniform float _Blend2;

const vec3 reflectColor = vec3(1.0);

void main() 
{	
	float fresnelFactor = dot(normalize(toCam), vec3(0.0, 1.0, 0.0));
	vec2 uv = fs_surface.texcoord * _Tiling;
    uv.y += 0.01 * (sin(uv.x * 3.5 + _Time * 0.35) + sin(uv.x * 4.8 + _Time * 1.05) + sin(uv.x * 7.3 + _Time * 0.45)) / 3.0;
    uv.x += 0.12 * (sin(uv.y * 4.0 + _Time * 0.5) + sin(uv.y * 6.8 + _Time * 0.75) + sin(uv.y * 11.3 + _Time * 0.2)) / 3.0;
    uv.y += 0.12 * (sin(uv.x * 4.2 + _Time * 0.64) + sin(uv.x * 6.3 + _Time * 1.65) + sin(uv.x * 8.2 + _Time * 0.45)) / 3.0;

	vec4 sample1 = texture(_MainTex, uv * vec2(1.0));
	vec4 sample2 = texture(_MainTex,uv * vec2(0.8));
	vec3 color = _WaterColor + vec3(sample1.r * _Blend1 - sample2.r * _Blend2);
	FragColor = vec4(color, 1.0);

	//float fresnelFactor = (dot(normalize(toCam), vec3(0.0, 1.0, 0.0)) - 1.0) * -0.5;
	//FragColor = vec4(texture(_MainTex, fs_surface.texcoord).rgb, fresnelFactor);
}