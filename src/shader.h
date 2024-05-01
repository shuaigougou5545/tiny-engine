#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;


class Shader
{
public:
    unsigned int ID;
    std::string shader_name;
    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    Shader(const char* vertexPath, const char* fragmentPath)
    {
        fs::path fs_path(vertexPath);
        shader_name = fs_path.stem().string();
        // 1. retrieve the vertex/fragment source code from filePath
        std::string vertexCode;
        std::string fragmentCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;
        // ensure ifstream objects can throw exceptions:
        vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        try 
        {
            // open files
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            std::stringstream vShaderStream, fShaderStream;
            // read file's buffer contents into streams
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            // close file handlers
            vShaderFile.close();
            fShaderFile.close();
            // convert stream into string
            vertexCode   = vShaderStream.str();
            fragmentCode = fShaderStream.str();
        }
        catch (std::ifstream::failure& e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
        }
        const char* vShaderCode = vertexCode.c_str();
        const char * fShaderCode = fragmentCode.c_str();
        // 2. compile shaders
        unsigned int vertex, fragment;
        // vertex shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");
        // fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");
        // shader Program
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");
        // delete the shaders as they're linked into our program now and no longer necessary
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }
    // activate the shader
    // ------------------------------------------------------------------------
    void use() 
    { 
        glUseProgram(ID); 
    }
    // get uniform location
    // ------------------------------------------------------------------------
    GLint getLocation(const std::string& name) const
    {
        GLint location = glGetUniformLocation(ID, name.c_str());
        if(location == -1)
        {
            // 静默处理
            // std::cout << "[ERROR]: Uniform " << name << " not found in shader " << shader_name << std::endl;
        }  
        return location;
    }
    // utility uniform functions
    // ------------------------------------------------------------------------
    void setBool(const std::string &name, bool value) const
    {         
        GLint location = getLocation(name);
        if(location != -1) {
            glUniform1i(location, (int)value); 
        } 
    }
    // ------------------------------------------------------------------------
    void setInt(const std::string &name, int value) const
    { 
        GLint location = getLocation(name);
        if(location != -1) {
            glUniform1i(location, value); 
        }
    }
    // ------------------------------------------------------------------------
    void setFloat(const std::string &name, float value) const
    { 
        GLint location = getLocation(name);
        if(location != -1) {
            glUniform1f(location, value); 
        }
    }
    // ------------------------------------------------------------------------
    void setVec3(const std::string &name, glm::vec3 value) const
    {
        GLint location = getLocation(name);
        if(location != -1) {
            glUniform3f(location, value.x, value.y, value.z);
        }
    }
    // ------------------------------------------------------------------------
    void setMat4(const std::string &name, glm::mat4 value) const
    {
        GLint location = getLocation(name);
        if(location != -1) {
            glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
        }
    }

private:
    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    void checkCompileErrors(unsigned int shader, std::string type)
    {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << " | " << "shader_name:" << shader_name << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << " | " << "shader_name:" << shader_name << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }
};

class ShaderLibrary {
public:
    void add(const std::string& name, Shader& shader) {
        shader_dict.emplace(name, std::make_shared<Shader>(shader));
        shader_names.push_back(name);
    }

    std::shared_ptr<Shader> get(const std::string& name) {
        auto it = shader_dict.find(name);
        if (it != shader_dict.end()) {
            return it->second;
        } else {
            throw std::runtime_error("Shader not found: " + name);
        }
    }

    void loadShaders(const std::string& file_path = "../resources/shaders/universal shaders")
    {
        // collect obj files from "../resources/shaders/universal shaders"
        std::string path = file_path;  

        try {
            if (fs::exists(path) && fs::is_directory(path)) {
                for (const auto& entry : fs::directory_iterator(path)) {
                    if (entry.is_directory()) {
                        std::string name = entry.path().stem().string();
                        std::string filename = entry.path().string();

                        std::string vsFilename;
                        std::string fsFilename;

                        std::string sub_path = entry.path().string();
                        for (const auto& sub_entry : fs::directory_iterator(sub_path)) {
                            if(sub_entry.is_directory()) 
                                continue;

                            std::string extension = sub_entry.path().extension().string();
                            if (extension == ".vert") {
                                vsFilename = sub_entry.path().string();
                            }
                            else if (extension == ".frag") {
                                fsFilename = sub_entry.path().string();
                            }
                        }

                        if (vsFilename.empty() || fsFilename.empty()) {
                            std::cout << "[ShaderLibrary]: " << "Skipping directory " << name << " because it does not contain both a vertex and fragment shader." << std::endl;
                            continue;
                        }

                        shader_dict.emplace(name, std::make_shared<Shader>(vsFilename.c_str(), fsFilename.c_str()));
                        shader_names.push_back(name);
                    }
                }
            }
        } catch (fs::filesystem_error& e) {
            std::cerr << e.what() << std::endl;
        }
    }
    
    std::unordered_map<std::string, std::shared_ptr<Shader>> shader_dict;
    std::vector<std::string> shader_names;
};

#endif // SHADER_H