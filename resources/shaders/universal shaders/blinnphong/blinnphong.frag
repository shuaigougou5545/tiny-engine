#version 330 core

in vec3 FragPos;
in vec3 NormalW;
in vec2 TexCoord;

out vec4 FragColor;

struct Material {
    vec3 albedo;
    vec3 fresnelR0;
    float roughness;
};

vec3 SchlickFresnel(vec3 R0, vec3 normal, vec3 lightDir)
{
    float c = max(dot(normal, lightDir), 0.0);
    float f0 = 1.0 - c;
    return R0 + (1.0 - R0) * pow(f0, 5.0);
}

void main()
{
    
}
