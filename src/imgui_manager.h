#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <vector>
#include <string>


class ImguiManager {
public:
    ImguiManager(GLFWwindow* w);
    ~ImguiManager();

    // 封装一些常用imgui组件
    void createCombo(const std::string& name, const std::vector<std::string>& items, int& selectedItem);

    GLFWwindow* window;
};