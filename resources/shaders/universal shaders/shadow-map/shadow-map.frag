#version 330 core
in float v_Depth; // light space

out vec4 FragColor;

void main()
{
    float z = v_Depth * 0.5 + 0.5; // [-1,1] -> [0,1]
    FragColor = vec4(vec3(z), 1.0);
}