#ifndef PROBE_H
#define PROBE_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <string>
#include <iostream>
#include "debug.h"
#include "model.h"
#include "shader.h"
#include "light.h"
#include "fbo.h"
#include "harmonics.h"


class Probe {
public:
    Probe(glm::vec3 pos, const std::string& probe_name = "probe", int width = 512, int height = 512); 

    void render(const ModelManagerList& model_manager_list, std::shared_ptr<Shader> shader, std::shared_ptr<Light> light); // 会改变视口
    void drawLightProbeFromCubeMap(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& scale = glm::vec3(0.2f)); // Probe可视化
    void drawLightProbeFromSH(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& scale = glm::vec3(0.2f)); // Probe可视化

    int bake_width = 256;
    int bake_height = 256;
    glm::vec3 center;
    std::vector<std::vector<unsigned char>> images; // render() -> cubemap

    const glm::vec3 cubemapDirections[6] = {
        {1, 0, 0},   // posx
        {-1, 0, 0},  // negx
        {0, 1, 0},    // posy
        {0, -1, 0},  // negy
        {0, 0, 1},   // posz 
        {0, 0, -1}, // negz 
    };

    const glm::vec3 cubemapUp[6] = {
        {0, 1, 0},   // posx
        {0, 1, 0},  // negx
        {0, 0, 1},    // posy
        {0, 0, -1},  // negy
        {0, 1, 0},   // posz
        {0, 1, 0}, // negz
    };

    // 渲染数据 => Cubemap
    void reverseU(std::vector<unsigned char>& image, int width, int height); // front back left right
    void reverseV(std::vector<unsigned char>& image, int width, int height); // top bottom

    std::shared_ptr<Shader> draw_shader; // Cube Map
    std::shared_ptr<Shader> prt_shader; // SH
    std::shared_ptr<ModelManager> model_manager; // 可视化sphere

    std::string name = "probe";
    const std::string folder_name = "../resources/texture/Baked-CubeMap/";

    Harmonics SH;
    std::vector<glm::mat3> coeffs;
};

#endif 