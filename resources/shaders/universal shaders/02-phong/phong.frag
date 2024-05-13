#version 330 core

in vec3 v_NormalW;
in vec3 v_PosW;

out vec4 FragColor;

uniform vec3 u_CameraPos;
uniform vec3 u_Albedo;
uniform float u_Shiness;
uniform vec3 u_LightPos;
uniform vec3 u_LightStrength;
uniform vec3 u_LightDirection;
uniform float u_LightInnerCutoff; // min angle - spot light
uniform float u_LightOuterCutoff; // max angle - spot light
uniform float u_LightRange; // max distance - spot light
uniform float u_Kdis;

float light_attenuation(vec3 frag_pos)
{
    // spot light
    vec3 L = normalize(u_LightPos - frag_pos); 
    float theta = dot(-L, normalize(u_LightDirection));
    float dis = length(frag_pos - u_LightPos);
    float epsilon = u_LightInnerCutoff - u_LightOuterCutoff;

    return smoothstep(u_LightRange, 0, dis) * clamp((theta - u_LightOuterCutoff) / epsilon, 0.0, 1.0);
}

void main()
{
    vec3 albedo = pow(u_Albedo, vec3(2.2));

    vec3 normalW = normalize(v_NormalW);
    vec3 L = normalize(u_LightPos - v_PosW);
    vec3 V = normalize(u_CameraPos - v_PosW);
    vec3 R = reflect(-L, normalW);

    // float attenuation = 1.0;
    // float attenuation = u_Kdis / (dis * dis);
    float attenuation = light_attenuation(v_PosW);
    vec3 radiance = u_LightStrength * attenuation; // Li

    // ambient
    vec3 ambient_light = vec3(1.0);
    vec3 ambient = ambient_light * 0.01;

    // diffuse
    float diff = max(dot(normalW, L), 0.0);
    vec3 diffuse = radiance * albedo * diff;

    // specular
    float c = max(dot(normalW, R), 0.0);
    float spec = c * c * c * c * c;
    vec3 specular = radiance * albedo * spec * u_Shiness;

    vec3 color = ambient + diffuse + specular;

    color = pow(color, vec3(1.0/2.2));
    FragColor = vec4(color, 1.0);
}