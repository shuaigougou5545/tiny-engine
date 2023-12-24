#include <vector>
#include <string>
#include <unordered_map>
#include "shader.h"

class ShaderLibrary {
public:
    void add(const std::string& name, Shader& shader) {
        shader_dict.emplace(name, shader);
        shader_names.push_back(name);
    }

    Shader& get(const std::string& name) {
        auto it = shader_dict.find(name);
        if (it != shader_dict.end()) {
            return it->second;
        } else {
            throw std::runtime_error("Shader not found: " + name);
        }
    }
    
    std::unordered_map<std::string, Shader> shader_dict;
    std::vector<std::string> shader_names;
};