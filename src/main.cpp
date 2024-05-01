#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>

#include "debug.h"
#include "model.h"
#include "shader.h"
#include "camera.h"
#include "light.h"
#include "fbo.h"
#include "utils.h"
#include "imgui_manager.h"

const int screen_width = 1080; // 800
const int screen_height = 720; // 600

//
// camera
Camera camera(glm::vec3(0.0f, 0.0f, 15.0f)); // 

// 
// point light
glm::vec3 light_pos = glm::vec3(0.0f, 1.5f, 4.f);
glm::vec3 light_strength = glm::vec3(1.0f, 1.0f, 1.0f);
LightProjectFunc project_func = PERSPECTIVE;
Light light(light_pos, light_strength, project_func, glm::vec2(screen_width, screen_height));

float lastX = screen_width / 2.0f;
float lastY = screen_height / 2.0f;
bool firstMouse = true;
bool leftMouseButtonPressed = false;
bool mouseOnImguiWindow = false;

//
// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

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

    GLFWwindow* window = glfwCreateWindow(screen_width, screen_height, "LearnOpenGL", NULL, NULL);
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
    // imgui

    ImguiManager imgui_manager(window);

    //
    // resource

    ModelLibrary model_library;
    model_library.loadModels();

    ModelBox skybox;

    int width, height, channels;
    unsigned int texture = Utils::loadTexture("../resources/texture/container.jpg", width, height, channels);
    unsigned int skybox_texture = Utils::loadCubeMap("../resources/texture/skybox", width, height, channels);

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
    boxl_manager->albedo = glm::vec3(0.0, 1.0, 0.0);
    boxr_manager->albedo = glm::vec3(1.0, 0.0, 0.0);
    
    ModelBox light_box("light_box");
    light_box.pos = light.Position;
    light_box.scale = glm::vec3(0.1);
    std::shared_ptr<ModelManager> light_manager = std::make_shared<ModelManager>(light_box);

    ModelQuad quad("quad");
    std::shared_ptr<ModelManager> quad_manager = std::make_shared<ModelManager>(quad);

    //
    // init

    std::shared_ptr<ModelManager> model_manager = std::make_shared<ModelManager>();
    std::shared_ptr<ModelManager> skybox_manager = std::make_shared<ModelManager>(skybox);

    //
    // shader

    std::shared_ptr<Shader> gbuffer_light_shader = std::make_shared<Shader>("../resources/shaders/gbuffer-light/gbuffer-light.vert", "../resources/shaders/gbuffer-light/gbuffer-light.frag");
    std::shared_ptr<Shader> skybox_shader = std::make_shared<Shader>("../resources/shaders/skybox/skybox.vert", "../resources/shaders/skybox/skybox.frag");


    ShaderLibrary shader_library;
    shader_library.loadShaders();
    std::shared_ptr<Shader> texture_shader = shader_library.get("texture");
    texture_shader->use();
    glBindTexture(GL_TEXTURE_2D, texture);

    //
    // G-Buffer
    // gbuffer_light: depth normalW posW flux
    std::vector<std::string> color_attachments = {"NormalW", "PosW", "Flux"};
    Fbo gbuffer_light_fbo {screen_width * 2, screen_height * 2, color_attachments};

    //
    // opengl status

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    //
    // render list
    std::vector<std::shared_ptr<Shader>> shader_list(2);
    shader_list[0] = gbuffer_light_shader;

    //
    // render objects
    ModelManagerList model_manager_list;
    model_manager_list.add(model_manager);
    model_manager_list.add(boxl_manager);
    model_manager_list.add(boxr_manager);
    model_manager_list.add(boxb_manager);
    model_manager_list.add(boxf_manager);
    model_manager_list.add(boxt_manager);
    model_manager_list.add(light_manager);


    while(!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        glfwPollEvents();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        gbuffer_light_fbo.use();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        gbuffer_light_fbo.unuse();

        //
        // imgui

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        ImGui::Begin("Tiny Engine");

        static int curr_model_index = 0;
        imgui_manager.createCombo("Model", model_library.model_names, curr_model_index);
        auto curr_model = model_library.get(model_library.model_names[curr_model_index]);
        model_manager->reloadModel(*curr_model);

        static int curr_shader_index = 9;
        imgui_manager.createCombo("Shader", shader_library.shader_names, curr_shader_index);
        auto curr_shader = shader_library.get(shader_library.shader_names[curr_shader_index]);

        shader_list[1] = curr_shader;

        bool button = false;
        imgui_manager.createButton("Capture", button);

        static glm::vec3 albedo = glm::vec3(1.000, 0.753, 0.796);
        imgui_manager.createSliderFloat3("Albedo", albedo);
        model_manager->albedo = albedo;

        static glm::vec3 pos = model_manager->model->pos;
        imgui_manager.createSliderFloat3("Pos", pos, -5.0f, 5.0f);
        model_manager->model->pos = pos;

        static float roughness = 0.3f;
        imgui_manager.createSliderFloat("Roughness", roughness);
        model_manager->roughness = roughness;

        static float metallic = 0.7f;
        imgui_manager.createSliderFloat("Metallic", metallic);
        model_manager->metallic = metallic;

        static float k_dis = 100.0f;
        imgui_manager.createSliderFloat("k_dis", k_dis, 1.0, 500.0);

        static float ao = 0.2f;
        imgui_manager.createSliderFloat("ao", ao);

        // Camera
        imgui_manager.createTextFloat3("Camera Position", camera.Position);

        // Light
        imgui_manager.createSliderFloat3("LightPos", light_manager->model->pos, -5.f, 5.f);
        light.Position = light_manager->model->pos;

        glm::vec2 nf = {light.near_z, light.far_z};
        imgui_manager.createSliderFloat2("LightNearFar", nf, 0.1f, 100.0f);
        light.near_z = nf.x;
        light.far_z = nf.y;

        static float fovy = 45.f;
        imgui_manager.createSliderFloat("LightFovy", fovy, 1.0f, 120.0f);
        light.fovy = glm::radians(fovy);

        light.update();

        ImGui::End();
        

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
                
                curr_shader->setMat4("u_Model", model_matrix);
                curr_shader->setMat4("u_View", view_matrix);
                curr_shader->setMat4("u_Projection", projection_matrix);
                // Camera
                curr_shader->setVec3("u_CameraPos", camera.Position);
                // Material
                curr_shader->setVec3("u_Albedo", curr_model_manager->albedo);
                curr_shader->setFloat("u_Roughness", curr_model_manager->roughness);
                curr_shader->setFloat("u_Metallic", curr_model_manager->metallic);
                // Light
                curr_shader->setVec3("u_LightPos", light.Position);
                curr_shader->setVec3("u_LightStrength", light.Strength);
                curr_shader->setMat4("u_LightViewMatrix", light.view_matrix);
                curr_shader->setMat4("u_LightProjectionMatrix", light.projection_matrix);
                curr_shader->setFloat("u_LightNear", light.near_z);
                curr_shader->setFloat("u_LightFar", light.far_z);
                // Texture
                curr_shader->setInt("u_Texture0", 0);
                // other
                curr_shader->setFloat("u_Kdis", k_dis);
                curr_shader->setFloat("u_Ao", ao);
            }

            //
            // render
            //

            //
            // step 1: G-Buffer
            shader_list[0]->use();
            gbuffer_light_fbo.use();
            curr_model_manager->draw();
            gbuffer_light_fbo.unuse();

            
            //
            // step 2: render normally

            // special case
            shader_list[1]->use();
            if(shader_list[1]->shader_name == "phong-shadow") {
                glBindTexture(GL_TEXTURE_2D, gbuffer_light_fbo.depth_texture_id);
                curr_model_manager->draw();
            } else if(shader_list[1]->shader_name == "rsm") {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, gbuffer_light_fbo.depth_texture_id);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, gbuffer_light_fbo.color_texture_ids[0]);
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, gbuffer_light_fbo.color_texture_ids[1]);
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, gbuffer_light_fbo.color_texture_ids[2]);
                shader_list[1]->setInt("u_ReflectorNormalW", 1);
                shader_list[1]->setInt("u_ReflectorPosW", 2);
                shader_list[1]->setInt("u_ReflectorFlux", 3);
                curr_model_manager->draw();
            } else if(shader_list[1]->shader_name == "quad") {
                quad_manager->draw();
            } else {
                curr_model_manager->draw();
            } 


        }

        
        if(button) {
            for(int i = 0; i < gbuffer_light_fbo.getColorAttachmentsNum(); ++i) {
                std::string output_filename = "../resources/output/location_" + std::to_string(i) + "_" + gbuffer_light_fbo.getColorAttachmentsName(i) + "_4.png";
                Utils::readFrameBufferColorAttachmentToPng(gbuffer_light_fbo.id, output_filename, screen_width * 2.0, screen_height * 2.0, 4, true, i);
            }
            Utils::readFrameBufferDepthAttachmentToPng(gbuffer_light_fbo.id, "../resources/output/depth_1.png", screen_width * 2.0, screen_height * 2.0);
        }

        // skybox render

        // skybox_shader->use();
        // skybox_shader->setMat4("view", glm::mat4(glm::mat3(view_matrix)));
        // skybox_shader->setMat4("projection", projection_matrix);
        // glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture);
        // glDepthFunc(GL_LEQUAL);
        // glBindVertexArray(skybox_manager->VAO);
        // glDrawElements(GL_TRIANGLES, skybox_manager->model->indices.size(), GL_UNSIGNED_INT, 0);
        // glDepthFunc(GL_LESS);

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