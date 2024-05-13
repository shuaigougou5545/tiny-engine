#version 330 core
layout(location = 0) in vec3 aPos;

out vec3 v_Dir;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;
uniform vec3 u_CenterW;

void main()
{
    vec4 posW = u_Model * vec4(aPos, 1.0);
    v_Dir = normalize(posW.xyz - u_CenterW);
    gl_Position = u_Projection * u_View * u_Model * vec4(aPos, 1.0);
}