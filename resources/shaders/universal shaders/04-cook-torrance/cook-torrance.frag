#version 330 core

in vec3 v_NormalW;
in vec3 v_PosW;
in vec4 v_PosH;
in vec2 v_TexCoord;

out vec4 FragColor;

uniform vec3 u_CameraPos;
uniform vec3 u_Albedo;
uniform float u_Metallic;
uniform float u_Roughness;
uniform vec3 u_LightPos;
uniform vec3 u_LightStrength;
uniform vec3 u_LightDirection;
uniform float u_LightInnerCutoff; // min angle - spot light
uniform float u_LightOuterCutoff; // max angle - spot light
uniform float u_LightRange; // max distance - spot light

// 可调节参数
uniform float u_Kdis; // 调节光线传播过程中的衰减

const float PI = 3.1415926;

float light_attenuation(vec3 frag_pos)
{
    // spot light
    vec3 L = normalize(u_LightPos - frag_pos); 
    float theta = dot(-L, normalize(u_LightDirection));
    float dis = length(frag_pos - u_LightPos);
    float epsilon = u_LightInnerCutoff - u_LightOuterCutoff;

    return smoothstep(u_LightRange, 0, dis) * clamp((theta - u_LightOuterCutoff) / epsilon, 0.0, 1.0);
}

// F: Fresnel - Schilick Approximation
vec3 FresnelSchlick(vec3 F0, vec3 V, vec3 H)
{
    float c = 1.0 - dot(V, H);
    return F0 + (vec3(1.0) - F0) * c * c * c * c * c;
}

// D: normal distribution function - GGX NDF
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float nom = roughness * roughness;
    float NoH = max(dot(N, H), 0.0);
    float denom = (NoH * NoH * (nom - 1.0) + 1.0);
    denom = PI * denom * denom;
    return nom / denom;
}

// G: geometry - Smith_Schlick_GGX
float GeometrySchlickGGX(float NoV, float k)
{
    // G1
    float nom = NoV;
    float denom = NoV * (1.0 - k) + k;
    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    // G = G1(n,i)·G1(n,v)
    return GeometrySchlickGGX(max(dot(N, V), 0.0), roughness) * GeometrySchlickGGX(max(dot(N, L), 0.0), roughness);
}

void main()
{
    vec3 albedo = pow(u_Albedo, vec3(2.2));
    float dis = length(v_PosW - u_LightPos);

    float attenuation = u_Kdis / dis; // considering gamma correction
    // float attenuation = light_attenuation(v_PosW);
    // float attenuation = 1.0;
    vec3 radiance = u_LightStrength * attenuation; // Li

    vec3 N = normalize(v_NormalW);
    vec3 L = normalize(u_LightPos - v_PosW);
    vec3 V = normalize(u_CameraPos - v_PosW);
    vec3 H = normalize(L + V);
    
    float NoV = max(dot(N, V), 0.0);
    float NoL = max(dot(N, L), 0.0);

    vec3 F0 = vec3(0.04); // 电解质默认f0:0.04
    F0 = mix(F0, albedo, u_Metallic); // 计算金属部分的F0

    vec3 Lo = vec3(0.0);

    // brdf_specular
    float NDF = DistributionGGX(N, H, u_Roughness);
    float G = GeometrySmith(N, V, L, u_Roughness);
    vec3 F = FresnelSchlick(F0, V, H);

    vec3 nom = NDF * G * F;
    float denom = max((4.0 * NoL * NoV), 0.001);
    vec3 BRDF = nom / denom; 

    // brdf_diffuse
    vec3 ks = F; // specular占比
    vec3 kd = vec3(1.0) - ks; // diffuse占比
    kd *= (1.0 - u_Metallic);

    Lo += (kd * u_Albedo / PI + BRDF) * radiance * NoL; // brdf_diffuse + brdf_specular

    vec3 color = Lo;

    // ambient
    vec3 ambient = vec3(0.03) * u_Albedo * 0.01;
    color += ambient;

    color = color / (color + vec3(1.0)); // tone mapping
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(color, 1.0);
}
