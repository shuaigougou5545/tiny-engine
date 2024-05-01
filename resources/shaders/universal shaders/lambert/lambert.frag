#version 330 core

in vec3 v_NormalW;
in vec3 v_PosW;

out vec4 FragColor;

uniform vec3 u_CameraPos;
uniform vec3 u_LightPos;
uniform vec3 u_LightStrength;
uniform vec3 u_Albedo;

void main()
{
    vec3 normalW = normalize(v_NormalW);
    vec3 L = normalize(u_LightPos - v_PosW);

    // ambient
    vec3 ambient_light = vec3(1.0);
    vec3 ambient = ambient_light * 0.01;

    // diffuse
    float diff = max(dot(normalW, L), 0.0);
    vec3 diffuse = u_LightStrength * u_Albedo * diff;

    FragColor = vec4(ambient + diffuse, 1.0);
}