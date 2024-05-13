#ifndef SAMPLE_H
#define SAMPLE_H

#include <iostream>
#include <string>
#include <glad/glad.h>
#include <vector>
#include <random>
#include "stb_image.h"
#include "stb_image_write.h"

class NoiseTexture {
public:
    int tex_width = 1080;
    int tex_height = 720;

    GLuint texture_id;

    NoiseTexture(int w, int h) : tex_width(w), tex_height(h)
    {
        std::random_device rd;  
        std::mt19937 gen(rd()); 
        std::uniform_real_distribution<> dis(0.0, 1.0); 
        
        const int channels = 3;
        std::vector<float> randomData(tex_width * tex_height * channels);
        for (auto& v : randomData) {
            v = static_cast<float>(dis(gen)); 
        }

        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // 1 channel
        // glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, tex_width, tex_height, 0, GL_RED, GL_FLOAT, randomData.data());
        
        // 3 channels
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, tex_width, tex_height, 0, GL_RGB, GL_FLOAT, randomData.data());

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    ~NoiseTexture() {
        glDeleteTextures(1, &texture_id);
    }
};

#endif // SAMPLE_H