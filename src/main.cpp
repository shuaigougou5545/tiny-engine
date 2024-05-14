#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>
#include <unordered_set>

#include "debug.h"
#include "model.h"
#include "shader.h"
#include "camera.h"
#include "light.h"
#include "fbo.h"
#include "utils.h"
#include "imgui_manager.h"
#include "sample.h"
#include "harmonics.h"
#include "probe.h"
#include "skybox.h"

const int screen_width = 1080; // 800
const int screen_height = 720; // 600

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));  // (glm::vec3(8.f), glm::vec3(0.0f)); 

float lastX = screen_width / 2.0f;
float lastY = screen_height / 2.0f;
bool firstMouse = true;
bool leftMouseButtonPressed = false;
bool mouseOnImguiWindow = false;

//
// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;
double lastTime = 0.0f;
int nbFrames = 0;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(screen_width, screen_height, "", NULL, NULL);
    if(window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glfwSetWindowSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    //
    // data

    // spot light
    Light light;
    light.type = POINT_LIGHT;
    light.perspective_frustum.aspect_ratio = float(screen_width) / float(screen_height);
    light.perspective_frustum.fovy = glm::radians(45.f);

    // for test
    light.position = glm::vec3(0.f, 2.f, 5.f);
    light.direction = glm::vec3(0.f) - light.position;

    // light.position = glm::vec3(1.0f, -2.0f, 0.f);
    // light.direction = glm::vec3(-1.f, 0.f, 0.f);

    // 角落
    // light.position = glm::vec3(8.f);
    // light.direction = glm::vec3(-1.f);

    light.outer_cutoff = glm::radians(40.f);

    // light.outer_cutoff = glm::radians(20.0f);
    light.strength = glm::vec3(1.f, 1.f, 1.f);
    light.updateMatrix();
    
    //
    // imgui

    ImguiManager imgui_manager(window);

    //
    // resource

    ModelLibrary model_library;
    model_library.loadModels();

    ModelSphere sphere;
    // sphere.scale = glm::vec3(0.f); // for test
    model_library.add(sphere.model_name, sphere);

    ModelBox skybox;

    int width, height, channels;
    unsigned int texture = Utils::loadTexture("../resources/texture/container.jpg", width, height, channels);

    SkyboxLibrary skybox_library;
    skybox_library.loadSkybox();

    ModelBox box_l("left"), box_r("right"), box_b("bottom"), box_f("front"), box_t("top");
    float thick = 0.1f;
    float radius = 3.f;
    float y_offset = 0.5f;
    float offset = 0.05f;
    box_l.pos = glm::vec3(-radius-offset, y_offset, 0.f);
    box_l.scale = glm::vec3(thick, 2.f*radius, 2.f*radius);
    box_r.pos = glm::vec3(radius+offset, y_offset, 0.f);
    box_r.scale = glm::vec3(thick, 2.f*radius, 2.f*radius);
    box_f.pos = glm::vec3(0.f, y_offset, -radius+offset);
    box_f.scale = glm::vec3(2.f*radius, 2.f*radius, thick);
    box_b.pos = glm::vec3(0.f, -radius+y_offset+offset, 0.f);
    box_b.scale = glm::vec3(2.f*radius, thick, 2.f*radius);
    box_t.pos = glm::vec3(0.f, radius+y_offset-offset, 0.f);
    box_t.scale = glm::vec3(2.f*radius, thick, 2.f*radius);
    std::shared_ptr<ModelManager> boxl_manager = std::make_shared<ModelManager>(box_l);
    std::shared_ptr<ModelManager> boxr_manager = std::make_shared<ModelManager>(box_r);
    std::shared_ptr<ModelManager> boxf_manager = std::make_shared<ModelManager>(box_f);
    std::shared_ptr<ModelManager> boxb_manager = std::make_shared<ModelManager>(box_b);
    std::shared_ptr<ModelManager> boxt_manager = std::make_shared<ModelManager>(box_t);
    
    // boxl_manager->albedo = glm::vec3(1.0, 0.0, 0.0); // 左
    // boxf_manager->albedo = glm::vec3(0.0, 1.0, 0.0); // 前
    // boxb_manager->albedo = glm::vec3(0.0, 0.0, 1.0); // 底
    
    // cornell box
    boxl_manager->albedo = glm::vec3(0.0, 1.0, 0.0); 
    boxr_manager->albedo = glm::vec3(1.0, 0.0, 0.0);

    // boxf_manager->albedo = glm::vec3(1.0, 1.0, 1.0);
    // boxb_manager->albedo = glm::vec3(1.0, 1.0, 0.0);
    // boxt_manager->albedo = glm::vec3(0.0, 1.0, 1.0);
    
    ModelBox light_box("light_box");
    light_box.pos = light.position;
    light_box.scale = glm::vec3(0.1);
    std::shared_ptr<ModelManager> light_manager = std::make_shared<ModelManager>(light_box);

    ModelQuad quad("quad");
    std::shared_ptr<ModelManager> quad_manager = std::make_shared<ModelManager>(quad);

    //
    // init

    std::shared_ptr<ModelManager> model_manager = std::make_shared<ModelManager>(sphere);
    std::shared_ptr<ModelManager> skybox_manager = std::make_shared<ModelManager>(skybox);

    //
    // shader

    std::shared_ptr<Shader> bake_shader = std::make_shared<Shader>("../resources/shaders/bake/bake.vert", "../resources/shaders/bake/bake.frag");
    std::shared_ptr<Shader> gbuffer_shader = std::make_shared<Shader>("../resources/shaders/gbuffer/gbuffer.vert", "../resources/shaders/gbuffer/gbuffer.frag");
    std::shared_ptr<Shader> gbuffer_light_shader = std::make_shared<Shader>("../resources/shaders/light-gbuffer/light-gbuffer.vert", "../resources/shaders/light-gbuffer/light-gbuffer.frag");
    std::shared_ptr<Shader> post_process_shader = std::make_shared<Shader>("../resources/shaders/post-process/post-process.vert", "../resources/shaders/post-process/post-process.frag");
    std::shared_ptr<Shader> skybox_shader = std::make_shared<Shader>("../resources/shaders/skybox/skybox.vert", "../resources/shaders/skybox/skybox.frag");


    ShaderLibrary shader_library;
    shader_library.loadShaders();

    //
    // G-Buffer
    // gbuffer: depth normalW
    std::vector<std::string> color_attachments_gbuffer = {"NormalW"};
    Fbo gbuffer_fbo {screen_width * 2, screen_height * 2, color_attachments_gbuffer, GL_CLAMP_TO_BORDER};

    // gbuffer_light: depth normalW posW flux
    std::vector<std::string> color_attachments_light_gbuffer = {"NormalW", "PosW", "Flux"};
    Fbo light_gbuffer_fbo {screen_width * 2, screen_height * 2, color_attachments_light_gbuffer, GL_CLAMP_TO_BORDER};

    // default
    std::vector<std::string> color_attachments_default = {"Color"};
    Fbo default_fbo {screen_width * 2, screen_height * 2, color_attachments_default, GL_CLAMP_TO_EDGE};

    //
    // opengl status
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    //
    // render list
    std::vector<std::shared_ptr<Shader>> shader_list(4);
    shader_list[0] = gbuffer_shader;
    shader_list[1] = gbuffer_light_shader;
    shader_list[3] = post_process_shader;

    //
    // render objects
    ModelManagerList model_manager_list;
    model_manager_list.add(model_manager);

    model_manager_list.add(boxl_manager);
    model_manager_list.add(boxb_manager);
    model_manager_list.add(boxf_manager);
    model_manager_list.add(boxr_manager);
    model_manager_list.add(boxt_manager);

    model_manager_list.add(light_manager);

    NoiseTexture noise_texture(screen_width, screen_height);

    //
    // Precompute
    Harmonics SH;
    // light
    for(auto& p : skybox_library.skybox_dict)
    {
        std::string name = p.first;
        auto& skybox = p.second;
        std::string filepath = skybox_library.getFilepath(name);

        // only compute once
        // auto coeffs_light = SH.computeCubemapSH(filepath); 
        // std::cout << name << ": " << coeffs_light.size() << std::endl;
    }

    // light transport
    std::unordered_set<std::string> objects_supported_SH;
    for(auto& p : model_library.model_dict)
    {
        std::string name = p.first;
        auto& model = p.second;
        
        // only compute once
        // auto coeffs_transport = SH.computeVerticesSH(*model); 
        // std::cout << name << ": " << coeffs_transport.size() << std::endl;

        objects_supported_SH.insert(name);
    }
    // light transport - cornell box
    auto box_manager_list = {boxl_manager, boxb_manager, boxf_manager, boxr_manager, boxt_manager};
    for(auto& box_manager : box_manager_list)
    {
        // only compute once
        // auto coeffs_transport = SH.computeVerticesSH(*(box_manager->model)); 
        // std::cout << box_manager->model->model_name << ": " << coeffs_transport.size() << std::endl;

        std::string filename = "../resources/obj/" + box_manager->model->model_name + "/transport.txt";
        box_manager->reloadModel(*(box_manager->model), filename);
        std::cout << filename << std::endl;
    }
   
    
    // return 0;

    const int r = 2;
    const float interval = 3.0f;
    std::vector<std::shared_ptr<Probe>> probes;
    probes.reserve(r * r * r);
    int index = 0;
    for(int i = 0; i < r; ++i)
    {
        for(int j = 0; j < r; ++j)
        {
            for(int k = 0; k < r; ++k)
            {
                std::string name = "probe" + std::to_string(++index);
                float offset = (r - 1) / 2.0f;
                glm::vec3 position((i - offset) * interval, (j - offset) * interval, (k - offset) * interval);
                probes.emplace_back(std::make_shared<Probe>(position, name));
            }
        }
    }

    while(!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        nbFrames++;

        if (currentFrame - lastTime >= 1.0) {
            std::stringstream title;
            title << "FPS: " << nbFrames << " - Frame Rate Display";
            glfwSetWindowTitle(window, title.str().c_str());

            nbFrames = 0;
            lastTime += 1.0;
        }

        processInput(window);
        glfwPollEvents();

        // clear framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        gbuffer_fbo.use();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        gbuffer_fbo.unuse();

        light_gbuffer_fbo.use();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        light_gbuffer_fbo.unuse();

        default_fbo.use();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        default_fbo.unuse();

        //
        // imgui

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        ImGui::Begin("Tiny Engine");

        static int curr_model_index = 1;
        imgui_manager.createCombo("Model", model_library.model_names, curr_model_index);
        auto curr_model = model_library.get(model_library.model_names[curr_model_index]);

        if(objects_supported_SH.count(curr_model->model_name)) {
            // SH
            model_manager->reloadModel(*curr_model, "../resources/obj/" + curr_model->model_name + "/transport.txt");
        } else {
            // traditional
            model_manager->reloadModel(*curr_model);
        }

        static int shader_index = 3; // for test
        static int curr_shader_index = shader_index;
        imgui_manager.createCombo("Shader", shader_library.shader_names, curr_shader_index);
        auto curr_shader = shader_library.get(shader_library.shader_names[curr_shader_index]);

        shader_list[2] = curr_shader;

        bool button = false;
        imgui_manager.createButton("Capture", button);

        static bool skybox_button = true;
        imgui_manager.createCheckbox("SkyboxEnabled", skybox_button);

        static int skybox_index = 0;
        imgui_manager.createCombo("Skybox", skybox_library.skybox_names, skybox_index);
        auto skybox_name = skybox_library.skybox_names[skybox_index];
        unsigned int skybox_texture = skybox_library.get(skybox_name);
        std::string skybox_filepath = skybox_library.getFilepath(skybox_name);

        static bool light_button = true;
        imgui_manager.createCheckbox("Light", light_button);
        static float shiness = 0.3f;
        static float k_dis = 100.0f;
        if(light_button) 
        {
            ImGui::Begin("Light");

            imgui_manager.createSliderFloat3("Light Position", light.position, -10.f, 10.f);
            light_manager->model->pos = light.position;

            imgui_manager.createSliderFloat3("Light Direction", light.direction, -1.f, 1.f);

            imgui_manager.createSliderFloat("Light Range", light.light_range, 1.f, 100.f);

            static float min_angle = glm::degrees(light.inner_cutoff);
            static float max_angle = glm::degrees(light.outer_cutoff);
            imgui_manager.createSliderFloat("InnerCutoffAngle", min_angle, 0.f, 90.f);
            imgui_manager.createSliderFloat("OuterCutoff", max_angle, 0.f, 90.f);
            light.inner_cutoff = glm::radians(min_angle);
            light.outer_cutoff = glm::radians(max_angle);

            imgui_manager.createSliderFloat("Shiness", shiness, 0.0f, 1.0f);
            imgui_manager.createSliderFloat("k_dis", k_dis, 1.0, 500.0);
            
            light.updateMatrix();

            ImGui::End();
        }

        static bool sample_button = true;
        imgui_manager.createCheckbox("Sample", light_button);
        static int sample_count = 1;
        static float sample_radius = 500.f;
        static glm::vec3 debug = glm::vec3(0.f);
        if(light_button) 
        {
            ImGui::Begin("Sample");

            imgui_manager.createSliderInt("sample_count", sample_count, 1, 300);
            imgui_manager.createSliderFloat("sample_radius", sample_radius, 1.0, 1000.0);
            imgui_manager.createInputFloat3("debug", debug);

            ImGui::End();
        }

        static bool post_process_button = true;
        imgui_manager.createCheckbox("Post Process", post_process_button);
        static int gaussian_radius = 3;
        static float sigmaI = 2.f;
        static float sigmaS = 10.f;
        if(post_process_button) 
        {
            ImGui::Begin("Post Process");

            imgui_manager.createSliderInt("gaussian_radius", gaussian_radius, 1, 10);
            imgui_manager.createSliderFloat("sigmaI", sigmaI, 0.0, 20.0);
            imgui_manager.createSliderFloat("sigmaS", sigmaS, 0.0, 20.0);
            
            
            ImGui::End();
        }
        glCheckError();

        static glm::vec3 albedo = glm::vec3(1.000, 0.753, 0.796);
        imgui_manager.createSliderFloat3("Albedo", albedo);
        model_manager->albedo = albedo;

        static glm::vec3 pos = model_manager->model->pos;
        imgui_manager.createSliderFloat3("Pos", pos, -5.0f, 5.0f);
        model_manager->model->pos = pos;

        static glm::vec3 scale = model_manager->model->scale;
        imgui_manager.createSliderFloat3("Scale", scale, 0.0f, 5.0f);
        model_manager->model->scale = scale;

        static float roughness = 0.3f;
        imgui_manager.createSliderFloat("Roughness", roughness);
        model_manager->roughness = roughness;

        static float metallic = 0.7f;
        imgui_manager.createSliderFloat("Metallic", metallic);
        model_manager->metallic = metallic;

        static float ao = 0.2f;
        imgui_manager.createSliderFloat("ao", ao);

        // Camera
        imgui_manager.createTextFloat3("Camera Position", camera.Position);

        static float randomness = 10.0f;
        imgui_manager.createSliderFloat("Randomness", randomness, 0.0f, 50.0f);

        ImGui::End();
        glCheckError();
        
        static bool first = true;
        if(first) {
            for(auto& probe_ptr : probes)
            {
                auto probe_manager_list = model_manager_list;
                probe_manager_list.get(0)->model->scale = glm::vec3(0.0f); // 中心物体不展示

                probe_ptr->render(probe_manager_list, bake_shader, std::make_shared<Light>(light));
            }
            glfwGetFramebufferSize(window, &width, &height);
            glViewport(0, 0, width, height);
            first = false;
        }
        // return 0;


        //
        // special case

        if(curr_shader->shader_name == "reflect")
        {
            curr_shader->use();
            curr_shader->setInt("skybox", 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture);
        }
        // 
        // render data

        // MVP
        
        glm::mat4 view_matrix = camera.GetViewMatrix();
        glm::mat4 projection_matrix = glm::perspective(glm::radians(camera.Zoom), (float)screen_width / (float)screen_height, 0.1f, 100.0f);

        for(int i = 0; i < model_manager_list.count(); ++i) {
            auto curr_model_manager = model_manager_list.get(i);
            auto curr_model = curr_model_manager->model;

            glm::mat4 model_matrix = glm::mat4(1.0f);
            model_matrix = glm::translate(model_matrix, curr_model->pos);
            model_matrix = glm::scale(model_matrix, curr_model->scale);
            

            for(int j = 0; j < shader_list.size(); ++j) {
                auto curr_shader = shader_list[j];
                curr_shader->use();

                // time
                curr_shader->setFloat("u_Time", (float)glfwGetTime());
                // resolution
                curr_shader->setVec2("u_Resolution", glm::vec2(float(screen_width), float(screen_height)));
                // mvp
                curr_shader->setMat4("u_Model", model_matrix);
                curr_shader->setMat4("u_View", view_matrix);
                curr_shader->setMat4("u_Projection", projection_matrix);
                // Camera
                curr_shader->setVec3("u_CameraPos", camera.Position);
                // Material
                curr_shader->setVec3("u_Albedo", curr_model_manager->albedo);
                curr_shader->setFloat("u_Roughness", curr_model_manager->roughness);
                curr_shader->setFloat("u_Metallic", curr_model_manager->metallic);
                curr_shader->setFloat("u_Shiness", shiness);
                // Light
                curr_shader->setVec3("u_LightPos", light.position);
                curr_shader->setVec3("u_LightDirection", light.direction);
                curr_shader->setVec3("u_LightStrength", light.strength);
                curr_shader->setMat4("u_LightViewMatrix", light.view_matrix);
                curr_shader->setMat4("u_LightProjectionMatrix", light.projection_matrix);
                curr_shader->setFloat("u_LightInnerCutoff", cos(light.inner_cutoff));
                curr_shader->setFloat("u_LightOuterCutoff", cos(light.outer_cutoff));
                curr_shader->setFloat("u_LightRange", light.light_range);
                // Sample
                curr_shader->setInt("u_SampleCount", sample_count);
                curr_shader->setFloat("u_SampleRadius", sample_radius);
                // Texture
                curr_shader->setInt("u_Texture0", 0);
                // Post Process
                curr_shader->setInt("u_GaussianRadius", gaussian_radius);
                curr_shader->setFloat("u_SigmaI", sigmaI);
                curr_shader->setFloat("u_SigmaS", sigmaS);
                // other
                curr_shader->setFloat("u_Kdis", k_dis);
                curr_shader->setFloat("u_Ao", ao);
                curr_shader->setFloat("u_Randomness", randomness);
                curr_shader->setVec3("u_Debug", debug);
            }
            glCheckError();

            //
            // render
            //

            //
            // step 1: G-Buffer
            shader_list[0]->use();
            gbuffer_fbo.use();
            curr_model_manager->draw();
            gbuffer_fbo.unuse();
            glCheckError();
            
            //
            // step 2: Light-G-Buffer
            shader_list[1]->use();
            light_gbuffer_fbo.use();
            curr_model_manager->draw();
            light_gbuffer_fbo.unuse();
            glCheckError();

            
            //
            // step 3: render normally
            // default_fbo.use();

            // special case
            shader_list[2]->use();
            if(shader_list[2]->shader_name == "phong-shadow") {
                glBindTexture(GL_TEXTURE_2D, light_gbuffer_fbo.depth_texture_id);
                curr_model_manager->draw();
            } else if(shader_list[2]->shader_name == "rsm") {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, light_gbuffer_fbo.depth_texture_id);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, light_gbuffer_fbo.color_texture_ids[0]);
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, light_gbuffer_fbo.color_texture_ids[1]);
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, light_gbuffer_fbo.color_texture_ids[2]);
                shader_list[2]->setInt("u_ReflectorNormalW", 1);
                shader_list[2]->setInt("u_ReflectorPosW", 2);
                shader_list[2]->setInt("u_ReflectorFlux", 3);
                glActiveTexture(GL_TEXTURE4);
                glBindTexture(GL_TEXTURE_2D, noise_texture.texture_id);
                shader_list[2]->setInt("u_Noise", 4);
                curr_model_manager->draw();
            } else if(shader_list[2]->shader_name == "quad") {
                quad_manager->draw();
            } else if(shader_list[2]->shader_name == "prt") {
                auto L = Utils::loadLightSHFromTxt(skybox_filepath + "/light.txt");
                shader_list[2]->setMat3Array("u_PrecomputeL", L);
                curr_model_manager->draw();
            } else if(shader_list[2]->shader_name == "light-probe-gi"
                || shader_list[2]->shader_name == "light-probe-gi(fragment)") {
                std::string filepath = "../resources/texture/Baked-CubeMap/";
                auto L0 = Utils::loadLightSHFromTxt(filepath + "probe1" + "/light.txt");
                auto L1 = Utils::loadLightSHFromTxt(filepath + "probe2" + "/light.txt");
                auto L2 = Utils::loadLightSHFromTxt(filepath + "probe3" + "/light.txt");
                auto L3 = Utils::loadLightSHFromTxt(filepath + "probe4" + "/light.txt");
                shader_list[2]->setMat3Array("u_PrecomputeL0", L0);
                shader_list[2]->setMat3Array("u_PrecomputeL1", L1);
                shader_list[2]->setMat3Array("u_PrecomputeL2", L2);
                shader_list[2]->setMat3Array("u_PrecomputeL3", L3);
                shader_list[2]->setVec3("u_PosL0", probes[0]->center);
                shader_list[2]->setVec3("u_PosL1", probes[1]->center);
                shader_list[2]->setVec3("u_PosL2", probes[2]->center);
                shader_list[2]->setVec3("u_PosL3", probes[3]->center);
                curr_model_manager->draw();
            } else {
                curr_model_manager->draw();
            } 
            glCheckError();
            
            // default_fbo.unuse();
        }

        // 可视化Light Probe
        for(auto& probe_ptr : probes) {
            // probe_ptr->drawLightProbeFromCubeMap(view_matrix, projection_matrix);
            probe_ptr->drawLightProbeFromSH(view_matrix, projection_matrix);
        }
        
        
        //
        // step 4: post process
        // shader_list[3]->use();
        // glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_2D, default_fbo.color_texture_ids[0]);
        // glActiveTexture(GL_TEXTURE1);
        // glBindTexture(GL_TEXTURE_2D, gbuffer_fbo.depth_texture_id);
        // glActiveTexture(GL_TEXTURE2);
        // glBindTexture(GL_TEXTURE_2D, gbuffer_fbo.color_texture_ids[0]);
        // shader_list[3]->setInt("u_Texture0", 0);
        // shader_list[3]->setInt("u_GBufferDepth", 1);
        // shader_list[3]->setInt("u_GBufferNormalW", 2);
        // quad_manager->draw();
        
        
        if(button) {
            // auto& curr_fbo = light_gbuffer_fbo;
            // bool multi_attachments = curr_fbo.getColorAttachmentsNum() > 1;
            // for(int i = 0; i < curr_fbo.getColorAttachmentsNum(); ++i) {
            //     std::string output_filename = "../resources/output/location_" + std::to_string(i) + "_" + curr_fbo.getColorAttachmentsName(i) + "_4.png";
            //     Utils::readFrameBufferColorAttachmentToPng(curr_fbo.id, output_filename, screen_width * 2.0, screen_height * 2.0, 4, true, multi_attachments, i);
            // }
            // Utils::readFrameBufferDepthAttachmentToPng(curr_fbo.id, "../resources/output/depth_1.png", screen_width * 2.0, screen_height * 2.0);
            
            std::string file_name = "../resources/output/" + curr_shader->shader_name + ".png";
            Utils::readFrameBufferColorAttachmentToPng(0, file_name, screen_width * 2.0, screen_height * 2.0, 4, true, 0);
            Utils::readFrameBufferDepthAttachmentToPng(0, "../resources/output/depth.png", screen_width * 2.0, screen_height * 2.0);
        }

        // skybox render
        if(skybox_button) {
            skybox_shader->use();
            skybox_shader->setMat4("view", glm::mat4(glm::mat3(view_matrix)));
            skybox_shader->setMat4("projection", projection_matrix);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture);
            glDepthFunc(GL_LEQUAL);
            glBindVertexArray(skybox_manager->VAO);
            glDrawElements(GL_TRIANGLES, skybox_manager->model->indices.size(), GL_UNSIGNED_INT, 0);
            glDepthFunc(GL_LESS);
        }
        

        glCheckError();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
#ifdef __APPLE__
    // APPLE是视网膜显示器,帧缓冲区的长宽是窗口实际长宽的两倍
    glfwGetFramebufferSize(window, &width, &height);
#endif
    glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if(button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if(action == GLFW_PRESS)
        {
            leftMouseButtonPressed = true;
        }
        else if(action == GLFW_RELEASE)
        {
            leftMouseButtonPressed = false;
        }
    }
    
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse)
        mouseOnImguiWindow = true;
    else
        mouseOnImguiWindow = false;
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;
    
    if(leftMouseButtonPressed && !mouseOnImguiWindow){
        camera.ProcessMouseMovement(xoffset, yoffset);
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if(!mouseOnImguiWindow)
        camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}