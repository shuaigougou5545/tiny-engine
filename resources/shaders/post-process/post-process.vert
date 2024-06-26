#version 330 core
layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

out vec2 v_FragCoord;

void main()
{
    gl_Position = vec4(a_Pos, 1.0);

    v_FragCoord = a_TexCoord;
}