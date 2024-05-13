#version 330 core
layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

out vec3 v_NormalW;
out vec3 v_PosW;
out vec4 v_PosH;
out vec2 v_TexCoord;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{
    v_NormalW = mat3(transpose(inverse(u_Model))) * a_Normal;
    v_PosW = (u_Model * vec4(a_Pos, 1.0)).xyz;
    v_PosH = u_Projection * u_View * u_Model * vec4(a_Pos, 1.0);
    v_TexCoord = a_TexCoord;

    gl_Position = v_PosH;
}