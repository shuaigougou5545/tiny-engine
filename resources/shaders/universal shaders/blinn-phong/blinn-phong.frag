#version 330 core

in vec3 v_NormalW;
in vec3 v_PosW;

out vec4 FragColor;

uniform vec3 u_CameraPos;
uniform vec3 u_LightPos;
uniform vec3 u_LightStrength;
uniform vec3 u_Albedo;

float my_pow_5(float c)
{
    return c * c * c * c * c;
}
void main()
{
    vec3 normalW = normalize(v_NormalW);
    vec3 L = normalize(u_LightPos - v_PosW);
    vec3 V = normalize(u_CameraPos - v_PosW);
    vec3 H = normalize(L + V);

    // ambient
    vec3 ambient_light = vec3(1.0);
    vec3 ambient = ambient_light * 0.01;

    // diffuse
    float diff = max(dot(normalW, L), 0.0);
    vec3 diffuse = u_LightStrength * u_Albedo * diff;

    // specular
    float spec = my_pow_5(max(dot(normalW, H), 0.0));
    vec3 specular = u_LightStrength * u_Albedo * spec;

    FragColor = vec4(ambient + diffuse + specular, 1.0);
}