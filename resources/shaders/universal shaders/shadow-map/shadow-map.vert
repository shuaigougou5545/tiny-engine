#version 330 core
layout(location = 0) in vec3 a_Pos;

out float v_Depth;

uniform mat4 u_Model;
uniform mat4 u_LightViewMatrix;
uniform mat4 u_LightProjectionMatrix;

void main()
{
    gl_Position = u_LightProjectionMatrix * u_LightViewMatrix* u_Model * vec4(a_Pos, 1.0); // light space
    v_Depth = gl_Position.z / gl_Position.w;
}