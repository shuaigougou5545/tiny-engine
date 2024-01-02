#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "debug.h"
#include "model.h"
#include "shader.h"
#include "texture_loader.h"
#include "imgui_manager.h"

const int screen_width = 800;
const int screen_height = 600;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
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

    // imgui

    ImguiManager imgui_manager(window);

    // resource

    ModelLibrary model_library;
    model_library.loadModels();
    ModelBox box;
    model_library.add("box", box);


    ModelBox skybox;

    int width, height, channels;
    unsigned int texture = loadTexture("../resources/texture/container.jpg", width, height, channels);
    unsigned int skybox_texture = loadCubeMap("../resources/texture/skybox", width, height, channels);

    // init

    ModelManager model_manager;
    ModelManager skybox_manager(skybox);

    // shader

    ShaderLibrary shader_library;
    Shader normal_shader("../resources/shaders/normal.vert", "../resources/shaders/normal.frag");
    Shader texture_shader("../resources/shaders/texture.vert", "../resources/shaders/texture.frag");
    shader_library.add("normal", normal_shader);
    shader_library.add("texture", texture_shader);

    Shader skybox_shader("../resources/shaders/skybox.vert", "../resources/shaders/skybox.frag");

    texture_shader.use();
    texture_shader.setInt("texture_diffuse", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    // opengl status

    glEnable(GL_DEPTH_TEST);

    while(!glfwWindowShouldClose(window))
    {
        processInput(window);
        glfwPollEvents();

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // imgui

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        ImGui::Begin("Tiny Engine");

        static int curr_model_index = 0;
        imgui_manager.createCombo("Model", model_library.model_names, curr_model_index);
        auto curr_model = model_library.get(model_library.model_names[curr_model_index]);
        model_manager.reloadModel(*curr_model);

        static int curr_shader_index = 0;
        imgui_manager.createCombo("Shader", shader_library.shader_names, curr_shader_index);
        auto curr_shader = shader_library.get(shader_library.shader_names[curr_shader_index]);

        ImGui::End();

        // render

        curr_shader->use();
        glm::mat4 model_matrix = glm::mat4(1.0f);
        float theta = glm::radians(90.0f); // 俯仰角
        float phi = glm::radians(glfwGetTime() * 10.0); // 方位角
        float radius = 5.0f; // 半径
        glm::vec3 eye = glm::vec3(sin(theta) * cos(phi), cos(theta), sin(theta) * sin(phi)) * radius;
        glm::mat4 view_matrix = glm::lookAt(eye, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection_matrix = glm::perspective(glm::radians(45.0f), (float)screen_width / (float)screen_height, 0.1f, 100.0f);
        curr_shader->setMat4("model", model_matrix);
        curr_shader->setMat4("view", view_matrix);
        curr_shader->setMat4("projection", projection_matrix);
        model_manager.draw();

        // skybox render

        skybox_shader.use();
        skybox_shader.setMat4("view", glm::mat4(glm::mat3(view_matrix)));
        skybox_shader.setMat4("projection", projection_matrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture);
        glDepthFunc(GL_LEQUAL);
        glBindVertexArray(skybox_manager.VAO);
        glDrawElements(GL_TRIANGLES, skybox_manager.model->indices.size(), GL_UNSIGNED_INT, 0);
        glDepthFunc(GL_LESS);

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

void processInput(GLFWwindow* window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}