#version 330 core

in vec3 FragPos;
in vec3 NormalW;
in vec2 TexCoord;
out vec4 FragColor;

uniform samplerCube skybox;
uniform vec3 cameraPos;

void main()
{
    vec3 normalW = normalize(NormalW);
    vec3 viewDir = normalize(FragPos - cameraPos);
    vec3 reflect_dir = reflect(viewDir, normalW);
    FragColor = texture(skybox, reflect_dir);
}