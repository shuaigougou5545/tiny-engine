#version 330 core
layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec3 a_Normal;

out vec3 v_PosW;
out vec3 v_NormalW;
out vec4 v_PosH; // light space

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;
uniform mat4 u_LightViewMatrix;
uniform mat4 u_LightProjectionMatrix;

void main()
{
    vec4 posW = u_Model * vec4(a_Pos, 1.0);
    v_PosW = posW.xyz;
    v_NormalW = mat3(transpose(inverse(u_Model))) * a_Normal;
    
    v_PosH = u_LightProjectionMatrix * u_LightViewMatrix* u_Model * vec4(a_Pos, 1.0);
    gl_Position = v_PosH; // light ndc
}