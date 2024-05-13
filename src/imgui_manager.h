#ifndef IMGUI_MANAGER_H
#define IMGUI_MANAGER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <string>


class ImguiManager {
public:
    ImguiManager(GLFWwindow* w);
    ~ImguiManager();

    // 封装一些常用imgui组件
    void createCombo(const std::string& name, const std::vector<std::string>& items, int& selectedItem);
    void createButton(const std::string& name, bool& pressed);
    void createCheckbox(const std::string& name, bool& checked);
    void createSliderInt(const std::string& name, int& value, int min = 0, int max = 10);
    void createSliderFloat(const std::string& name, float& value, float min = 0.f, float max = 1.f);
    void createSliderFloat2(const std::string& name, glm::vec2& value, float min = 0.f, float max = 1.f);
    void createSliderFloat3(const std::string& name, glm::vec3& value, float min = 0.f, float max = 1.f);
    void createInputFloat3(const std::string& name, glm::vec3& value);
    void createTextFloat3(const std::string& name, glm::vec3& value);

    GLFWwindow* window;
};

#endif // IMGUI_MANAGER_H