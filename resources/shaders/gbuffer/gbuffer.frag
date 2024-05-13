#version 330 core
in vec3 v_NormalW;

layout(location = 0) out vec4 o_NormalW;

void main()
{
    o_NormalW = vec4(v_NormalW, 1.0);
}