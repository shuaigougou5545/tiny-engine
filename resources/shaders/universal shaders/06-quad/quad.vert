#version 330 core
layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

out vec4 v_PosNdc;
out vec2 v_TexCoord;

void main()
{
    gl_Position = vec4(a_Pos, 1.0);

    v_PosNdc = vec4(a_Pos, 1.0);
    v_TexCoord = a_TexCoord;
}