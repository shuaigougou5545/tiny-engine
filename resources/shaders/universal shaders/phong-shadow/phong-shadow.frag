#version 330 core

in vec3 v_NormalW;
in vec3 v_PosW;

out vec4 FragColor;

uniform vec3 u_CameraPos;
uniform vec3 u_LightPos;
uniform vec3 u_LightStrength;
uniform vec3 u_Albedo;
uniform mat4 u_LightViewMatrix;
uniform mat4 u_LightProjectionMatrix;

uniform sampler2D u_Texture0; // shadow map

float my_pow_5(float c)
{
    return c * c * c * c * c;
}

vec3 getLightSpacePos(vec3 posW)
{
    vec4 pos = u_LightProjectionMatrix * u_LightViewMatrix * vec4(posW, 1.0);
    pos /= pos.w;
    return pos.xyz * 0.5 + 0.5; // [0,1]
}

void main()
{
    vec3 normalW = normalize(v_NormalW);
    vec3 L = normalize(u_LightPos - v_PosW);
    vec3 V = normalize(u_CameraPos - v_PosW);
    vec3 R = reflect(-L, normalW);

    vec3 ndc = getLightSpacePos(v_PosW); 
    float depth = texture(u_Texture0, ndc.xy).r; // blocker
    float bias = 0.005;
    float visibility = (ndc.z - bias < depth) ? 1.0 : 0.0;

    // ambient
    vec3 ambient_light = vec3(1.0);
    vec3 ambient = ambient_light * 0.01;

    // diffuse
    float diff = max(dot(normalW, L), 0.0);
    vec3 diffuse = u_LightStrength * u_Albedo * diff;

    // specular
    float spec = my_pow_5(max(dot(normalW, R), 0.0));
    vec3 specular = u_LightStrength * u_Albedo * spec;

    vec3 color = ambient + diffuse + specular;
    color *= visibility;

    // color = vec3(visibility); // for test

    FragColor = vec4(color, 1.0);
}