#version 450

in vec3 out_Normal;
in vec3 toCam;

in Surface 
{
	vec2 texcoord;
} fs_surface;

out vec4 FragColor;

uniform sampler2D _MainTex;
uniform sampler2D _WaterWarpTex;
uniform sampler2D _WaterSpecTex;
uniform vec3 _WaterColor;
uniform float _Tiling;
uniform float _Time;
uniform float _Blend1;
uniform float _Blend2;
uniform float _TexScale;
uniform float _SpecScale;
uniform float _WarpScale;
uniform float _WarpStrength;
uniform float lowCuttoff;
uniform float upperCuttoff;

const vec3 reflectColor = vec3(1.0);

void main() 
{	
	vec2 uv = fs_surface.texcoord * _Tiling;
	vec2 warpScroll = vec2(0.5, 0.5) * _Time;
	vec2 warp = texture(_WaterWarpTex, uv + warpScroll * 0.5).xy * _WarpStrength;

	warp = (warp * 2.0) - 1.0;

	vec2 specUV = fs_surface.texcoord * _SpecScale;
	vec3 sample1 = texture(_WaterSpecTex, specUV + warpScroll * 0.2).rgb;
	vec3 sample2 = texture(_WaterSpecTex, specUV + warpScroll * 0.1).rgb;
	vec3 spec = sample1 + sample2;

	float brightness = dot(spec, vec3(0.299, 0.587, 0.114));
	if (brightness <= lowCuttoff || brightness >= upperCuttoff)
	{
		
	}

	float fresnelFactor = dot(normalize(toCam), vec3(0.0, 1.0, 0.0));

	vec4 color = texture(_MainTex, uv + warp + warpScroll);

	vec3 finalColor = mix(spec, _WaterColor + color.r, fresnelFactor);
	FragColor = vec4(finalColor, 1.0);

	//float fresnelFactor = (dot(normalize(toCam), vec3(0.0, 1.0, 0.0)) - 1.0) * -0.5;
	//FragColor = vec4(texture(_MainTex, fs_surface.texcoord).rgb, fresnelFactor);
}