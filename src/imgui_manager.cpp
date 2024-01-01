#include "imgui_manager.h"


ImguiManager::ImguiManager(GLFWwindow* w) : window(w)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

ImguiManager::~ImguiManager()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ImguiManager::createCombo(const std::string& name, const std::vector<std::string>& items, int& selectedItem)
{
    std::vector<const char*> cnames;
    for (const auto& item : items) {
        cnames.push_back(item.c_str());
    }
    ImGui::Combo(name.c_str(), &selectedItem, cnames.data(), cnames.size());
}