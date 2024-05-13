#version 330 core

in vec3 v_Dir;

out vec4 FragColor;

uniform samplerCube u_Cubemap;

void main()
{
    vec3 color = texture(u_Cubemap, normalize(v_Dir)).rgb;
    FragColor = vec4(color, 1.0);
}