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

uniform sampler2D u_Texture0; // shadow map
uniform sampler2D u_ReflectorNormalW;
uniform sampler2D u_ReflectorPosW;
uniform sampler2D u_ReflectorFlux;

float my_pow_5(float c)
{
    return c * c * c * c * c;
}

vec3 getLightSpacePos(vec3 posW)
{
    vec4 pos = u_LightProjectionMatrix * u_LightViewMatrix * vec4(posW, 1.0);
    pos /= pos.w;
    return pos.xyz; // [-1,1]
}

void main()
{
    // camera
    vec4 ndc = v_PosH / v_PosH.w; // [-1,1]
    vec2 fragCoord = ndc.xy * 0.5 + 0.5; // [0,1]

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

    vec3 color = ambient + diffuse + specular;

    FragColor = vec4(vec3(texture(u_Texture0, fragCoord).r), 1.0);
    FragColor = texture(u_ReflectorNormalW, fragCoord);
    FragColor = texture(u_ReflectorPosW, fragCoord);
    FragColor = texture(u_ReflectorFlux, fragCoord);
}