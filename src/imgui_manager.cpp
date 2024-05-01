#include "imgui_manager.h"
#include <vector>


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

 void ImguiManager::createButton(const std::string& name, bool& pressed)
 {
    pressed = ImGui::Button(name.c_str());
 }

 void ImguiManager::createSliderFloat(const std::string& name, float& value, float min, float max)
 {
    ImGui::SliderFloat(name.c_str(), &value, min, max);
 }

 void ImguiManager::createInputFloat3(const std::string& name, glm::vec3& value)
 {
    std::vector<float> temp = {value.x, value.y, value.z};
    ImGui::InputFloat3(name.c_str(), temp.data());
    value.x = temp[0];
    value.y = temp[1];
    value.z = temp[2];
 }

 void ImguiManager::createSliderFloat2(const std::string& name, glm::vec2& value, float min, float max)
 {
	std::vector<float> temp = {value.x, value.y};
	ImGui::SliderFloat2(name.c_str(), temp.data(), min, max);
	value.x = temp[0]; 
	value.y = temp[1];
 }

 void ImguiManager::createSliderFloat3(const std::string& name, glm::vec3& value, float min, float max)
 {
    std::vector<float> temp = {value.x, value.y, value.z};
    ImGui::SliderFloat3(name.c_str(), temp.data(), min, max);
    value.x = temp[0]; 
    value.y = temp[1];
    value.z = temp[2];
 }

 void ImguiManager::createTextFloat3(const std::string& name, glm::vec3& value)
 {
    ImGui::Text("%s: (%.1f, %.1f, %.1f)", name.c_str(), value.x, value.y, value.z);
 }