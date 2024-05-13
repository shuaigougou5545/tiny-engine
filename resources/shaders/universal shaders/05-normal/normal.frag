#version 330 core

in vec3 v_Normal;
in vec2 v_TexCoord;
in vec4 v_PosH;

out vec4 FragColor;

void main()
{
    vec3 n = normalize(v_Normal) * 0.5 + 0.5; // [-1,1]=>[0,1]
    FragColor = vec4(n, 1.0);
}