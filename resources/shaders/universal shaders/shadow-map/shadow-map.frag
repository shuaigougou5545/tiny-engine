#version 330 core
in vec4 v_PosH; // light space

out vec4 FragColor;

void main()
{
    vec4 ndc = v_PosH / v_PosH.w;
    float z = ndc.z * 0.5 + 0.5; // [-1,1] -> [0,1]
    FragColor = vec4(vec3(z), 1.0);
}