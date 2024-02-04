#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "debug.h"
#include "model.h"
#include "shader.h"
#include "camera.h"
#include "utils.h"
#include "imgui_manager.h"

const int screen_width = 800;
const int screen_height = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
float lastX = screen_width / 2.0f;
float lastY = screen_height / 2.0f;
bool firstMouse = true;
bool leftMouseButtonPressed = false;
bool mouseOnImguiWindow = false;

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

    // imgui

    ImguiManager imgui_manager(window);

    // resource

    ModelLibrary model_library;
    model_library.loadModels();
    ModelBox box;
    model_library.add("box", box);


    ModelBox skybox;

    int width, height, channels;
    unsigned int texture = Utils::loadTexture("../resources/texture/container.jpg", width, height, channels);
    unsigned int skybox_texture = Utils::loadCubeMap("../resources/texture/skybox", width, height, channels);

    // init

    ModelManager model_manager;
    ModelManager skybox_manager(skybox);

    // shader

    Shader skybox_shader("../resources/shaders/skybox/skybox.vert", "../resources/shaders/skybox/skybox.frag");

    ShaderLibrary shader_library;
    shader_library.loadShaders();
    std::shared_ptr<Shader> texture_shader = shader_library.get("texture");
    texture_shader->use();
    texture_shader->setInt("texture_diffuse", 0);
    glBindTexture(GL_TEXTURE_2D, texture);

    // opengl status

    glEnable(GL_DEPTH_TEST);

    while(!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

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

        // special case

        if(curr_shader->shader_name == "reflect")
        {
            curr_shader->use();
            curr_shader->setVec3("cameraPos", camera.Position);
            curr_shader->setInt("skybox", 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture);
        }

        // render

        curr_shader->use();
        glm::mat4 model_matrix = glm::mat4(1.0f);
        glm::mat4 view_matrix = camera.GetViewMatrix();
        glm::mat4 projection_matrix = glm::perspective(glm::radians(camera.Zoom), (float)screen_width / (float)screen_height, 0.1f, 100.0f);
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