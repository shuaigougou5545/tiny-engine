#version 330 core
layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
// light transport
layout(location = 3) in vec3 a_PrecomputeLT0; 
layout(location = 4) in vec3 a_PrecomputeLT1; 
layout(location = 5) in vec3 a_PrecomputeLT2; 

out vec3 v_Color;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;
// light(rgb)
uniform mat3 u_PrecomputeL[3]; 

float L_dot_LT(mat3 L, mat3 LT) 
{
    return dot(L[0], LT[0]) + dot(L[1], LT[1]) + dot(L[2], LT[2]);
}

void main()
{
    mat3 LT = mat3(a_PrecomputeLT0, a_PrecomputeLT1, a_PrecomputeLT2);

    for(int i = 0; i < 3; ++i)
    {
        v_Color[i] = L_dot_LT(u_PrecomputeL[i], LT);
    }

    gl_Position = u_Projection * u_View * u_Model * vec4(a_Pos, 1.0);
}