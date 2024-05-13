#version 330 core

in vec3 v_Color;

out vec4 FragColor;

void main()
{
    vec3 color = v_Color;
    FragColor = vec4(color, 1.0);
}
