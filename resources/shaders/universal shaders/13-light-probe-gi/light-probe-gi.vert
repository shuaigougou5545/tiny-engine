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
// 4 probes 3 channels 9 coordinates
uniform mat3 u_PrecomputeL0[3]; 
uniform mat3 u_PrecomputeL1[3]; 
uniform mat3 u_PrecomputeL2[3]; 
uniform mat3 u_PrecomputeL3[3]; 
uniform vec3 u_PosL0;
uniform vec3 u_PosL1;
uniform vec3 u_PosL2;
uniform vec3 u_PosL3;


float L_dot_LT(mat3 L, mat3 LT) 
{
    return dot(L[0], LT[0]) + dot(L[1], LT[1]) + dot(L[2], LT[2]);
}

vec4 computeBarycentricCoordinates(vec3 p, vec3 a, vec3 b, vec3 c, vec3 d)
{
    // 重心坐标插值 -> 返回四个位置的权重
    vec3 vap = p - a;
    vec3 vbp = p - b;
    vec3 vcp = p - c;
    vec3 vab = b - a;
    vec3 vac = c - a;
    vec3 vad = d - a;
    vec3 vbc = c - b;
    vec3 vbd = d - b;
    vec3 vcd = d - c;
    float va6 = dot(vap, cross(vbc, vbd));
    float vb6 = dot(vbp, cross(vbd, vad));
    float vc6 = dot(vcp, cross(vad, vab));
    float vd6 = dot(vap, cross(vab, vac));
    float sum = va6 + vb6 + vc6 + vd6;
    return vec4(va6 / sum, vb6 / sum, vc6 / sum, vd6 / sum);
}

void main()
{
    mat3 LT = mat3(a_PrecomputeLT0, a_PrecomputeLT1, a_PrecomputeLT2);

    vec3 posW = (u_Model * vec4(a_Pos, 1.0)).xyz;
    vec4 baryCoords = computeBarycentricCoordinates(posW, u_PosL0, u_PosL1, u_PosL2, u_PosL3);

    mat3 interpolatedL[3];
    interpolatedL[0] = baryCoords.x * u_PrecomputeL0[0] + baryCoords.y * u_PrecomputeL1[0] + baryCoords.z * u_PrecomputeL2[0] + baryCoords.w * u_PrecomputeL3[0];
    interpolatedL[1] = baryCoords.x * u_PrecomputeL0[1] + baryCoords.y * u_PrecomputeL1[1] + baryCoords.z * u_PrecomputeL2[1] + baryCoords.w * u_PrecomputeL3[1];
    interpolatedL[2] = baryCoords.x * u_PrecomputeL0[2] + baryCoords.y * u_PrecomputeL1[2] + baryCoords.z * u_PrecomputeL2[2] + baryCoords.w * u_PrecomputeL3[2];

    for(int i = 0; i < 3; ++i)
    {
        v_Color[i] = L_dot_LT(interpolatedL[i], LT);
    }

    gl_Position = u_Projection * u_View * u_Model * vec4(a_Pos, 1.0);
}