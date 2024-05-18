#version 330 core

in vec3 v_PosW;
in vec3 v_NormalW;
in vec3 v_PrecomputeLT0;
in vec3 v_PrecomputeLT1;
in vec3 v_PrecomputeLT2;

out vec4 FragColor;

// 4 probes 3 channels 9 coordinates
uniform mat3 u_PrecomputeL0[3]; 
uniform mat3 u_PrecomputeL1[3]; 
uniform mat3 u_PrecomputeL2[3]; 
uniform mat3 u_PrecomputeL3[3]; 
uniform vec3 u_PosL0;
uniform vec3 u_PosL1;
uniform vec3 u_PosL2;
uniform vec3 u_PosL3;

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
uniform float u_Indirect;


float L_dot_LT(mat3 L, mat3 LT) 
{
    return dot(L[0], LT[0]) + dot(L[1], LT[1]) + dot(L[2], LT[2]);
}

vec4 computeBarycentricCoordinates(vec3 p, vec3 a, vec3 b, vec3 c, vec3 d)
{
    // 重心坐标插值 -> 返回四个位置的权重
    vec3 vap = p - a;
    vec3 vbp = p - b;
    vec3 vcp = p - c;
    vec3 vab = b - a;
    vec3 vac = c - a;
    vec3 vad = d - a;
    vec3 vbc = c - b;
    vec3 vbd = d - b;
    vec3 vcd = d - c;
    float va6 = dot(vap, cross(vbc, vbd));
    float vb6 = dot(vbp, cross(vbd, vad));
    float vc6 = dot(vcp, cross(vad, vab));
    float vd6 = dot(vap, cross(vab, vac));
    float sum = va6 + vb6 + vc6 + vd6;
    return vec4(va6 / sum, vb6 / sum, vc6 / sum, vd6 / sum);
}

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
    //
    // indirect
    //
    mat3 LT = mat3(v_PrecomputeLT0, v_PrecomputeLT1, v_PrecomputeLT2);

    vec3 posW = normalize(v_PosW);
    vec4 baryCoords = computeBarycentricCoordinates(posW, u_PosL0, u_PosL1, u_PosL2, u_PosL3);

    mat3 interpolatedL[3];
    interpolatedL[0] = baryCoords.x * u_PrecomputeL0[0] + baryCoords.y * u_PrecomputeL1[0] + baryCoords.z * u_PrecomputeL2[0] + baryCoords.w * u_PrecomputeL3[0];
    interpolatedL[1] = baryCoords.x * u_PrecomputeL0[1] + baryCoords.y * u_PrecomputeL1[1] + baryCoords.z * u_PrecomputeL2[1] + baryCoords.w * u_PrecomputeL3[1];
    interpolatedL[2] = baryCoords.x * u_PrecomputeL0[2] + baryCoords.y * u_PrecomputeL1[2] + baryCoords.z * u_PrecomputeL2[2] + baryCoords.w * u_PrecomputeL3[2];

    vec3 indirect = vec3(0.0);

    for(int i = 0; i < 3; ++i)
    {
        indirect[i] = L_dot_LT(interpolatedL[i], LT);
    }

    //
    // direct
    //
    vec3 albedo = pow(u_Albedo, vec3(2.2));

    vec3 normalW = normalize(v_NormalW);
    vec3 L = normalize(u_LightPos - v_PosW);
    vec3 V = normalize(u_CameraPos - v_PosW);
    vec3 R = reflect(-L, normalW);

    float dis = length(u_LightPos - v_PosW);
    // float attenuation = 1.0;
    float attenuation = u_Kdis / (dis * dis);
    // float attenuation = light_attenuation(v_PosW);
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

    vec3 direct = ambient + diffuse + specular;

    vec3 color = direct + indirect * u_Indirect;

    color = pow(color, vec3(1.0/2.2));
    FragColor = vec4(color, 1.0);
}




