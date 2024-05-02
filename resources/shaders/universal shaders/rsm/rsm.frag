#version 330 core

in vec3 v_NormalW;
in vec3 v_PosW;
in vec4 v_PosH;

out vec4 FragColor;

uniform vec3 u_CameraPos;
uniform vec3 u_LightPos;
uniform vec3 u_LightStrength;
uniform vec3 u_Albedo;
uniform mat4 u_LightViewMatrix;
uniform mat4 u_LightProjectionMatrix;
uniform float u_LightNear;
uniform float u_LightFar;
uniform float u_Ao;
uniform sampler2D u_Texture0; // shadow map
uniform sampler2D u_ReflectorNormalW;
uniform sampler2D u_ReflectorPosW;
uniform sampler2D u_ReflectorFlux;

const int SAMPLE_COUNT = 100;
const float PI = 3.1415926;

float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

float my_pow_5(float c)
{
    return c * c * c * c * c;
}

vec4 getLightSpacePosNDC(vec3 posW)
{
    vec4 pos = u_LightProjectionMatrix * u_LightViewMatrix * vec4(posW, 1.0);
    pos /= pos.w;
    return pos; // [-1,1]
}

void main()
{
    // camera
    vec4 ndc = v_PosH / v_PosH.w; // [-1,1]
    vec2 fragCoord = ndc.xy * 0.5 + 0.5; // [0,1]

    // light
    vec4 ndc_light = getLightSpacePosNDC(v_PosW);
    vec2 uv_light = ndc_light.xy * 0.5 + 0.5;

    vec3 normalW = normalize(v_NormalW);
    vec3 L = normalize(u_LightPos - v_PosW);
    vec3 V = normalize(u_CameraPos - v_PosW);
    vec3 H = normalize(L + V);

    //
    // direct light
    //

    // ambient
    vec3 ambient = vec3(0.0);

    // diffuse
    float diff = max(dot(normalW, L), 0.0);
    vec3 diffuse = u_LightStrength * u_Albedo * diff;

    // specular
    float spec = my_pow_5(max(dot(normalW, H), 0.0));
    vec3 specular = u_LightStrength * u_Albedo * spec;

    //
    // indirect light
    //
    vec3 indirect = vec3(0.0);
    for(int i = 0; i < SAMPLE_COUNT; ++i)
    {
        float s1 = random(vec2(i * 20, i * 40));
        float s2 = random(vec2(i * 40, i * 20));
        vec2 random_offset = vec2(1.0f * s1 * sin(2.0 * PI * s2) + 1.0f * s1 * cos(2.0 * PI * s2));
        vec2 sampleuv = uv_light + random_offset;

        // secondary light source - q
        float q_depth = texture(u_Texture0, sampleuv).r;
        vec3 q_normalW = texture(u_ReflectorNormalW, sampleuv).xyz;
        vec3 q_posW = texture(u_ReflectorPosW, sampleuv).xyz;
        vec3 q_flux = texture(u_ReflectorFlux, sampleuv).xyz;

        vec3 pq = v_PosW - q_posW.xyz;
        float dis = length(pq);
        vec3 pq_norm = normalize(pq);

        float cos1 = max(dot(v_NormalW, pq_norm), 0.0);
        float cos2 = max(dot(q_normalW, -pq_norm), 0.0);
        
        vec3 curr_indirect_light = q_flux * cos1 * cos2 / dis;
        float weight = 1.0 / SAMPLE_COUNT;

        // indirect += curr_indirect_light;
        indirect += q_flux / SAMPLE_COUNT;
    }

    // vec3 color = ambient + diffuse + specular + indirect * u_Ao;
    vec3 color = indirect;

    FragColor = vec4(color, 1.0);
}