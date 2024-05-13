#version 330 core

in vec3 v_NormalW;
in vec3 v_PosW;
in vec4 v_PosH;

out vec4 FragColor;

uniform float u_Time;
uniform vec2 u_Resolution;
uniform vec3 u_CameraPos;
uniform float u_Kdis;
uniform vec3 u_Albedo;
uniform float u_Shiness;
uniform vec3 u_LightPos;
uniform vec3 u_LightStrength;
uniform vec3 u_LightDirection;
uniform float u_LightInnerCutoff; // min angle - spot light
uniform float u_LightOuterCutoff; // max angle - spot light
uniform float u_LightRange; // max distance - spot light
uniform mat4 u_LightViewMatrix;
uniform mat4 u_LightProjectionMatrix;
// light space
uniform sampler2D u_Texture0; // shadow map
uniform sampler2D u_ReflectorNormalW;
uniform sampler2D u_ReflectorPosW;
uniform sampler2D u_ReflectorFlux;
uniform sampler2D u_Noise; 

uniform int u_SampleCount;
uniform float u_SampleRadius;
// uniform float u_Randomness;

uniform vec3 u_Debug;

const float PI = 3.1415926;

vec2 hashIntToVec2(int n) {
    float n_float = float(n);
    float a = fract(sin(n_float) * 43758.5453123);
    float b = fract(cos(n_float) * 12345.54321);
    return vec2(a, b);
}

vec4 getLightSpacePosNDC(vec3 posW)
{
    vec4 pos = u_LightProjectionMatrix * u_LightViewMatrix * vec4(posW, 1.0);
    pos /= pos.w;
    return pos; // [-1,1]
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
    vec3 albedo = pow(u_Albedo, vec3(2.2));

    // resolution
    vec2 dxdy = 1.0 / u_Resolution;

    // camera
    vec4 ndc = v_PosH / v_PosH.w; // [-1,1]
    vec2 fragCoord = ndc.xy * 0.5 + 0.5; // [0,1]

    // light
    vec4 ndc_light = getLightSpacePosNDC(v_PosW);
    vec2 uv_light = ndc_light.xy * 0.5 + 0.5;

    vec3 N = normalize(v_NormalW);
    vec3 L = normalize(u_LightPos - v_PosW);
    vec3 V = normalize(u_CameraPos - v_PosW);
    vec3 H = normalize(L + V);

    vec3 color = vec3(0.0);

    //
    // indirect light
    //
    vec3 indirect = vec3(0.0);
    for(int i = 0; i < u_SampleCount; ++i)
    {
        vec2 offset = hashIntToVec2(i);
        float s1 = texture(u_Noise, fragCoord + offset).r;
        float s2 = texture(u_Noise, fragCoord + offset).g;

        // vec2 radius = u_SampleRadius * dxdy;
        // vec2 random_offset = vec2(radius.x * s1 * sin(2.0 * PI * s2) + radius.y * s1 * cos(2.0 * PI * s2));
        // float weight = s1 * s1;
        // vec2 sampleuv = uv_light + random_offset;

        vec2 sampleuv = uv_light + u_Debug.xy;


        // secondary light source - q
        // vec3 q_normalW = normalize(texture(u_ReflectorNormalW, sampleuv).xyz);
        vec3 q_posW = texture(u_ReflectorPosW, sampleuv).xyz;
        vec3 q_flux = texture(u_ReflectorFlux, sampleuv).xyz;

        // vec3 pq = v_PosW - q_posW; // p - q

        // float cos1 = max(dot(v_NormalW, -pq), 0.0); // θ
        // float cos2 = max(dot(q_normalW, pq), 0.0); // θ'

        // indirect += q_flux * cos1 * cos2 / pow(length(v_PosW - q_posW), 4.0) * weight;

        // indirect += q_flux;

        color = q_flux;
        color = vec3(uv_light + u_Debug.xy, 0.0);

        // if(sampleuv.x > 1.0 || sampleuv.x < 0.0 || sampleuv.y > 1.0 || sampleuv.y < 0.0)
        //     color = vec3(1.0);
    }
    // indirect = clamp(indirect / u_SampleCount, 0.0, 1.0);
    // color = indirect / u_SampleCount;

    //
    // direct light
    //
    // float attenuation = 1.0;
    // // float attenuation = u_Kdis / (dis * dis);
    // // float attenuation = light_attenuation(v_PosW);
    // vec3 radiance = u_LightStrength * attenuation; // Li

    // // diffuse
    // float diff = max(dot(N, L), 0.0);
    // vec3 diffuse = radiance * albedo * diff;

    // // specular
    // float c = max(dot(N, H), 0.0);
    // float spec = c * c * c * c * c;
    // vec3 specular = radiance * albedo * spec * u_Shiness;

    // vec3 direct = diffuse + specular;

    // color += direct;

    // color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, 1.0);
}