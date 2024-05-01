#version 330 core

in vec4 v_PosNdc;
in vec2 v_TexCoord;

out vec4 FragColor;

void main()
{
    vec4 ndc = v_PosNdc / v_PosNdc.w;
    vec2 uv = ndc.xy * 0.5 + 0.5;

    // FragColor = vec4(v_TexCoord, 0.0, 1.0);
    FragColor = vec4(uv, 0.0, 1.0);
}