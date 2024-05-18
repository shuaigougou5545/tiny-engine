#include "probe.h"
#include <ctime>
#include "utils.h"


Probe::Probe(glm::vec3 pos, const std::string& probe_name, int width, int height) : center(pos), name(probe_name), bake_width(width), bake_height(height)
{
    draw_shader = std::make_shared<Shader>("../resources/shaders/light-probe/light-probe.vert", "../resources/shaders/light-probe/light-probe.frag");
    prt_shader = std::make_shared<Shader>("../resources/shaders/prt/prt.vert", "../resources/shaders/prt/prt.frag");

    // load model
    ModelSphere sphere;
    model_manager = std::make_shared<ModelManager>(sphere, "../resources/obj/sphere/transport.txt"); // 注意路径
}

void Probe::render(const ModelManagerList& model_manager_list, std::shared_ptr<Shader> shader, std::shared_ptr<Light> light)
{
    // 从6个视角分别渲染场景,保存在cubemap中
    clock_t start_time = clock();
    std::cout << "[Probe]:  Baking Light Probe [name:" << name << "] ..." << std::endl;

    images.resize(6);
    const std::vector<std::string> names = {"posx", "negx", "posy", "negy", "posz", "negz" };

    for(int i = 0; i < 6; ++i)
    {
        Fbo bake_fbo {bake_width, bake_height, {"Color"}, GL_CLAMP_TO_EDGE};
        bake_fbo.use();
        glViewport(0, 0, bake_width, bake_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // viewport

        glm::vec3 direction = cubemapDirections[i];
        glm::vec3 up = cubemapUp[i];
        glm::mat4 view_matrix = glm::lookAt(center, center + direction, up);
        glm::mat4 projection_matrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f);

        for(int i = 0; i < model_manager_list.count(); ++i) {
            auto curr_model_manager = model_manager_list.get(i);
            auto curr_model = curr_model_manager->model;

            glm::mat4 model_matrix = glm::mat4(1.0f);
            model_matrix = glm::translate(model_matrix, curr_model->pos);
            model_matrix = glm::scale(model_matrix, curr_model->scale);
            
            shader->use();

            // mvp
            shader->setMat4("u_Model", model_matrix);
            shader->setMat4("u_View", view_matrix);
            shader->setMat4("u_Projection", projection_matrix);
            // Camera
            shader->setVec3("u_CameraPos", center);
            // Material
            shader->setVec3("u_Albedo", curr_model_manager->albedo);
            shader->setFloat("u_Roughness", curr_model_manager->roughness);
            shader->setFloat("u_Metallic", curr_model_manager->metallic);
            shader->setFloat("u_Shiness", 0.2f);
            // Light
            shader->setVec3("u_LightPos", light->position);
            shader->setVec3("u_LightDirection", light->direction);
            shader->setVec3("u_LightStrength", light->strength);
            shader->setMat4("u_LightViewMatrix", light->view_matrix);
            shader->setMat4("u_LightProjectionMatrix", light->projection_matrix);
            shader->setFloat("u_LightInnerCutoff", cos(light->inner_cutoff));
            shader->setFloat("u_LightOuterCutoff", cos(light->outer_cutoff));
            shader->setFloat("u_LightRange", light->light_range);
            shader->setFloat("u_Kdis", 40.0f);

            curr_model_manager->draw();
        }

        images[i].resize(bake_width * bake_height * 4);
        glReadPixels(0, 0, bake_width, bake_height, GL_RGBA, GL_UNSIGNED_BYTE, images[i].data());
        bake_fbo.unuse();


        // reverse image -> cubemap
        // 注意: fbo的颜色缓冲并没有被翻转
        switch (i)
        {
            case 0: case 1: case 4: case 5: {
                // front back left right
                reverseU(images[i], bake_width, bake_height);
                break;
            }
            case 2: case 3: {
                // top bottom
                reverseV(images[i], bake_width, bake_height);
                break;
            }
        }
        // 保存图片时会自动flip y
        Utils::saveTextureToJpg(images[i], folder_name + name + "/" + names[i] + ".jpg", bake_width, bake_height, 4);
        // 数据需要额外的flip y
        reverseV(images[i], bake_width, bake_height);
    }
    
    
    std::string SH_name = folder_name + name + "/light.txt"; // ../resources/texture/Baked-CubeMap/probe0/light.txt
    coeffs = SH.convertSH(SH.computeCubemapSH(images, bake_width, bake_height, 4, SH_name));

    clock_t end_time = clock();
    double elapsed_time = (end_time - start_time) / (double)CLOCKS_PER_SEC;
    std::cout << "[Probe]:  Time taken " << elapsed_time << " seconds." << std::endl;

    glCheckError();
}

void Probe::drawLightProbeFromCubeMap(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& scale)
{   
    // load cubemap
    unsigned int cubemap_texture;
    glGenTextures(1, &cubemap_texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_texture);
    
    unsigned char *data;  
    for(unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
            0, GL_RGBA, bake_width, bake_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, images[i].data()
        );
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // load shader
    draw_shader->use();
    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, center);
    model_matrix = glm::scale(model_matrix, scale);
    draw_shader->setMat4("u_Model", model_matrix);
    draw_shader->setMat4("u_View", view);
    draw_shader->setMat4("u_Projection", projection);
    draw_shader->setVec3("u_CenterW", center);
    draw_shader->setInt("u_Cubemap", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_texture);

    // draw
    model_manager->draw();
    glCheckError();
}

void Probe::drawLightProbeFromSH(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& scale)
{
    // load shader
    prt_shader->use();
    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, center);
    model_matrix = glm::scale(model_matrix, scale);
    prt_shader->setMat4("u_Model", model_matrix);
    prt_shader->setMat4("u_View", view);
    prt_shader->setMat4("u_Projection", projection);
    
    auto& L = coeffs;
    prt_shader->setMat3Array("u_PrecomputeL", L);

    // draw
    model_manager->draw();
    glCheckError();
}

void Probe::reverseU(std::vector<unsigned char>& image, int width, int height)
{
    // front back left right
    for (int y = 0; y < height; y++) { 
        for (int x = 0; x < width / 2; x++) { 
            int oppositeX = width - x - 1;
            int indexStart = y * width * 4 + x * 4;
            int indexEnd = y * width * 4 + oppositeX * 4;
            // 交换像素，假设每个像素由4个字节组成（RGBA）
            std::swap_ranges(&image[indexStart], &image[indexStart + 4], &image[indexEnd]);
        }
    }
}
void Probe::reverseV(std::vector<unsigned char>& image, int width, int height)
{
    // top bottom
    for (int y = 0; y < height / 2; y++) { 
        int oppositeY = height - y - 1;
        for (int x = 0; x < width; x++) {
            int indexStart = y * width * 4 + x * 4;
            int indexEnd = oppositeY * width * 4 + x * 4;
            // 交换像素，假设每个像素由4个字节组成（RGBA）
            std::swap_ranges(&image[indexStart], &image[indexStart + 4], &image[indexEnd]);
        }
    }
}