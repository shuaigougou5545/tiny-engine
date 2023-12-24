#version 330 core

out vec4 FragColor;
in vec3 Normal;
in vec2 TexCoord;

void main()
{
    vec3 n = normalize(Normal) * 0.5 + 0.5; // [-1,1]=>[0,1]
    FragColor = vec4(n, 1.0);
}