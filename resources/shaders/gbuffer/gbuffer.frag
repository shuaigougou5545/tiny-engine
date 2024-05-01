#version 330 core
in vec3 v_PosW;
in vec3 v_NormalW;
in vec4 v_PosH; // light space

layout(location = 0) out vec4 o_NormalW;
layout(location = 1) out vec4 o_PosW;
layout(location = 2) out vec4 o_Flux; // reflected flux: (Phi·fr)

uniform vec3 u_Albedo;
uniform vec3 u_LightPos;
uniform vec3 u_LightStrength;


void main()
{
    o_NormalW = vec4(v_NormalW, 1.0);
    o_PosW = vec4(v_PosW, 1.0);
    o_Flux = vec4(u_Albedo * u_LightStrength, 1.0);
}