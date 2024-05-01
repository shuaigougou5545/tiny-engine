#version 330 core
in vec4 v_PosH; // light space

out vec4 FragColor;

uniform float u_LightNear;
uniform float u_LightFar;

float linerizeDepth(float depth)
{
    // project back to linear system
    float z = depth * 2.0 - 1.0; // [0,1] -> [-1,1]
    return (2.0 * u_LightNear * u_LightFar) / (u_LightFar + u_LightNear - z * (u_LightFar - u_LightNear));
}

void main()
{
    vec4 ndc = v_PosH / v_PosH.w;
    float z = ndc.z * 0.5 + 0.5; // [-1,1] -> [0,1]

    // z = linerizeDepth(z);

    FragColor = vec4(vec3(z), 1.0);
}