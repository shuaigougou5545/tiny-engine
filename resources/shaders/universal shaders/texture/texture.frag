#version 330 core

in vec3 v_Normal;
in vec2 v_TexCoord;
out vec4 FragColor;
uniform sampler2D u_Texture0;

void main()
{
    FragColor = texture(u_Texture0, v_TexCoord);
}