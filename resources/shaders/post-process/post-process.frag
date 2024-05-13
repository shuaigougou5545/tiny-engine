#version 330 core

in vec2 v_FragCoord;

out vec4 FragColor;

uniform vec2 u_Resolution;
uniform sampler2D u_Texture0;
// screen space (G-Buffer)
uniform sampler2D u_GBufferDepth;
uniform sampler2D u_GBufferNormalW;
// Gaussian
uniform int u_GaussianRadius;
uniform float u_SigmaI; // 强度差异的高斯标准差
uniform float u_SigmaS; // 空间距离的高斯标准差

float gaussian(float x, float sigma) 
{
    return exp(-0.5 * x * x / (sigma * sigma));
}

void main()
{
    vec2 uv = v_FragCoord;

    // vec3 normal_central = texture(u_GBufferNormalW, uv).xyz;
    // float depth_central = texture(u_GBufferDepth, uv).r;

    // Bilateral Blur
    // int radius = u_GaussianRadius;

    // vec4 sum = vec4(0.0);
    // float weightSum = 0.0;
    // for (int i = -radius; i <= radius; i++) {
    //     for (int j = -radius; j <= radius; j++) {
    //         vec2 offset = vec2(i, j) / u_Resolution;
    //         vec3 normal = texture(u_GBufferNormalW, uv + offset).xyz;
    //         float depth = texture(u_GBufferDepth, uv + offset).r;

    //         float weightS = gaussian(length(vec2(i, j)), u_SigmaS);
    //         // float weightI = gaussian(distance(depth, depth_central), u_SigmaI) * gaussian(dot(normal, normal_central), u_SigmaI);
    //         float weightI = gaussian(distance(depth, depth_central), u_SigmaI) * pow(1.0 - dot(normal, normal_central), u_SigmaI);

    //         float weight = weightS * weightI;

    //         sum += texture(u_Texture0, uv + offset) * weight;
    //         weightSum += weight;
    //     }
    // }

    // FragColor = sum / weightSum;

    vec3 color = texture(u_Texture0, uv).rgb;
    FragColor = vec4(color, 1.0);
}