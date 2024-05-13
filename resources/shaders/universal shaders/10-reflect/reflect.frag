#version 330 core

in vec3 v_FragPos;
in vec3 v_NormalW;
in vec2 v_TexCoord;
out vec4 FragColor;

uniform samplerCube skybox;
uniform vec3 cameraPos;

void main()
{
    vec3 normalW = normalize(v_NormalW);
    vec3 viewDir = normalize(v_FragPos - cameraPos);
    vec3 reflect_dir = reflect(viewDir, normalW);
    FragColor = texture(skybox, reflect_dir);
}