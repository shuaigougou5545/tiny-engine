#include "utils.h"

#include <vector>
#include "stb_image.h"
#include "stb_image_write.h"


void Utils::saveTextureToPng(
    GLuint texture_id, 
    const std::string& filename, 
    int width, 
    int height, 
    int channels,
    bool flipped
)
{
    std::vector<unsigned char> data;
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
    stbi_write_png(filename.c_str(), width, height, channels, data.data(), width * channels);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_flip_vertically_on_write(flipped);
}

unsigned int Utils::loadTexture(const std::string& filename, int& width, int& height, int& channels) 
{
    // default texture format: GL_RGB
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 0);
    if(!data)
    {
        std::cout << "[Texture]: load texture " << filename << " failed!" << std::endl;
        return -1;
    }
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);

    std::cout << "[Texture]: load texture " << filename << " successfully!" << std::endl;

    glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}

unsigned int Utils::loadCubeMap(const std::string& filepath, int& width, int& height, int& channels)
{
    // default texture format: GL_RGB
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

    std::vector<std::string> suffixes = { "right", "left", "top", "bottom", "front", "back" };
    std::string extension = ".jpg";

    std::vector<std::string> textures_faces;
    for(auto& suffix : suffixes)
        textures_faces.push_back(filepath + "/" + suffix + extension);
    
    unsigned char *data;  
    for(unsigned int i = 0; i < textures_faces.size(); i++)
    {
        data = stbi_load(textures_faces[i].c_str(), &width, &height, &channels, 0);
        if(!data)
        {
            std::cout << "[Texture]: load cubemap texture " << textures_faces[i] << " failed!" << std::endl;
            return -1;
        }
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
            0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
        );
        stbi_image_free(data);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    std::cout << "[Texture]: load cubemap texture " << filepath << " successfully!" << std::endl;

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    return texture;
}