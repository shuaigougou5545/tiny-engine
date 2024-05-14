#ifndef SKYBOX_H
#define SKYBOX_H

#include <filesystem>
#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>
#include "debug.h"
#include "utils.h"


class SkyboxLibrary {
public:
    void add(const std::string& name, unsigned int id) {
        skybox_dict.emplace(name, id);
        skybox_names.push_back(name);
    }

    unsigned int get(const std::string& name) {
        auto it = skybox_dict.find(name);
        if (it != skybox_dict.end()) {
            return it->second;
        } else {
            throw std::runtime_error("[Skybox]: skybox not found: " + name);
        }
    }

    std::string getFilepath(const std::string& name) {
        auto it = skybox_filepath.find(name);
        if (it != skybox_filepath.end()) {
            return it->second;
        } else {
            throw std::runtime_error("[Skybox]: skybox not found: " + name);
        }
    }

    void loadSkybox(const std::string& file_path = "../resources/texture/")
    {
        // collect skybox(cubemap) files from "../resources/textures/"
        namespace fs = std::filesystem;
        std::string path = file_path;  

        try {
            if (fs::exists(path) && fs::is_directory(path)) {
                for (const auto& entry : fs::directory_iterator(path)) {
                    // 每个子目录是一个cubemap
                    if(entry.is_directory() && entry.path().stem().string() != "not-included") {
                        if(entry.path().stem().string() == "Baked-CubeMap") {
                            // Baked-Cubemap每个子目录的probe
                            for (const auto& sub_entry : fs::directory_iterator(entry))
                            {
                                if(sub_entry.is_directory()) {
                                    std::string name = sub_entry.path().stem().string();
                                    std::string filepath = sub_entry.path().string();
                                    int width, height, channels;
                                    unsigned int skybox_texture = Utils::loadCubeMap(sub_entry.path().string(), width, height, channels);
                                    skybox_names.push_back(name);
                                    skybox_filepath.emplace(name, filepath);
                                    skybox_dict.emplace(name, skybox_texture);
                                }
                            }
                        } else {
                            // 其他cubemap
                            std::string name = entry.path().stem().string();
                            std::string filepath = entry.path().string();
                            int width, height, channels;
                            unsigned int skybox_texture = Utils::loadCubeMap(entry.path().string(), width, height, channels);
                            skybox_names.push_back(name);
                            skybox_filepath.emplace(name, filepath);
                            skybox_dict.emplace(name, skybox_texture);
                        }
                    }
                }
            }
        } catch (fs::filesystem_error& e) {
            std::cerr << e.what() << std::endl;
        }
    }
    
    std::unordered_map<std::string, unsigned int> skybox_dict; // [name, texture_id]
    std::vector<std::string> skybox_names;
    std::unordered_map<std::string, std::string> skybox_filepath; // [name, filepath]
 };


#endif