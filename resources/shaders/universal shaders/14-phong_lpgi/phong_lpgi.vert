#version 330 core
layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
// light transport
layout(location = 3) in vec3 a_PrecomputeLT0; 
layout(location = 4) in vec3 a_PrecomputeLT1; 
layout(location = 5) in vec3 a_PrecomputeLT2; 

out vec3 v_PosW;
out vec3 v_NormalW;
out vec3 v_PrecomputeLT0;
out vec3 v_PrecomputeLT1;
out vec3 v_PrecomputeLT2;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{
    v_PosW = (u_Model * vec4(a_Pos, 1.0)).xyz;
    v_NormalW = mat3(transpose(inverse(u_Model))) * a_Normal;

    v_PrecomputeLT0 = a_PrecomputeLT0;
    v_PrecomputeLT1 = a_PrecomputeLT1;
    v_PrecomputeLT2 = a_PrecomputeLT2;
    gl_Position = u_Projection * u_View * u_Model * vec4(a_Pos, 1.0);
}