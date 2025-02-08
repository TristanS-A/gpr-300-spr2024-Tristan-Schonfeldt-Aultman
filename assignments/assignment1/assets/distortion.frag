#version 450

out vec4 FragColor;

in vec2 vs_textcoords;

uniform sampler2D _MainTex;
uniform float _Time;
uniform float _Speed;
uniform float _Strength;

vec2 computeNewUV(vec2 uv, float offset, float offsetCube)
{
    vec2 center = uv - 0.5;
    float radiusSQ = center.x * center.x + center.y * center.y;
    float distortionVal = 0.0f;

    if (offsetCube == 0.0)
    {
        distortionVal = 1.0 + radiusSQ * offset;
    }
    else 
    {
        distortionVal = 1.0 + radiusSQ * (offset + offsetCube * sqrt(radiusSQ));
    }

    vec2 newUV = distortionVal * center + 0.5;

    return newUV;
}

void main() 
{
    float offset =  _Strength * sin(_Time * _Speed);
    float offsetCube = 0.5 * sin(_Time);

    vec3 finalColor = texture(_MainTex, computeNewUV(vs_textcoords, offset, offsetCube)).rgb;

    FragColor = vec4(finalColor, 1.0);
}