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
    vec3 albedo = pow(u_Albedo, vec3(2.2));

    vec3 normalW = normalize(v_NormalW);
    vec3 L = normalize(u_LightPos - v_PosW);

    // ambient
    vec3 ambient_light = vec3(1.0);
    vec3 ambient = ambient_light * 0.01;

    // diffuse
    float diff = max(dot(normalW, L), 0.0);
    vec3 diffuse = u_LightStrength * albedo * diff;

    vec3 color = ambient + diffuse;

    color = pow(color, vec3(1.0/2.2));
    FragColor = vec4(color, 1.0);
}