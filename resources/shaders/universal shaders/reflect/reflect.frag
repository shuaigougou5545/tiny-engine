#version 330 core

in vec3 NormalW;
in vec2 TexCoord;
out vec4 FragColor;

// uniform samplerCube skybox;

void main()
{
    vec3 normalW = normalize(NormalW);
    FragColor = vec4(normalW, 1.0);
}