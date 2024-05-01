#version 330 core
layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec3 a_Normal;

out vec3 v_NormalW;
out vec3 v_PosW;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{
    gl_Position = u_Projection * u_View * u_Model * vec4(a_Pos, 1.0);

    v_NormalW = mat3(transpose(inverse(u_Model))) * a_Normal;
    v_PosW = (u_Model * vec4(a_Pos, 1.0)).xyz;
}