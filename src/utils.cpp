#include "utils.h"

#include <vector>
#include "stb_image.h"
#include "stb_image_write.h"
#include "debug.h"


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
    data.resize(width * height * channels);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_flip_vertically_on_write(flipped);
    stbi_write_png(filename.c_str(), width, height, channels, data.data(), width * channels);
    glCheckError();
    std::cout << "[Utils]: save texture[id:" << texture_id << " ] to png: " << filename << " successfully!" << std::endl;
}

void Utils::readFrameBufferColorAttachmentToPng(
    GLuint fbo_id,
    const std::string& filename,
    int width,
    int height,
    int channels,
    bool flipped,
    int attachment_num
)
{
    std::vector<unsigned char> data;
    data.resize(width * height * channels);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
    glReadBuffer(GL_COLOR_ATTACHMENT0 + attachment_num);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    stbi_flip_vertically_on_write(flipped);
    stbi_write_png(filename.c_str(), width, height, channels, data.data(), width * channels);
    std::cout << "[Utils]: read framebuffer[id:" << fbo_id <<  "] color to png: " << filename << " successfully!" << std::endl;
}

void Utils::readFrameBufferDepthAttachmentToPng(
    GLuint fbo_id,
    const std::string& filename,
    int width,
    int height,
    bool flipped
)
{
    std::vector<unsigned char> data(width * height);
    std::vector<GLfloat> temp(width * height);
    
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
    glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, temp.data());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    for (size_t i = 0; i < data.size(); ++i) {
        // 0~1 => 0~255
        unsigned char depthValue = static_cast<unsigned char>(temp[i] * 255.0f);
        data[i] = depthValue;
    }

    stbi_flip_vertically_on_write(flipped);
    stbi_write_png(filename.c_str(), width, height, 1, data.data(), width);
    std::cout << "[Utils]: read framebuffer[id:" << fbo_id <<  "] depth to png: " << filename << " successfully!" << std::endl;
}

unsigned int Utils::loadTexture(
    const std::string& filename, 
    int& width, 
    int& height, 
    int& channels
)
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

unsigned int Utils::loadCubeMap(
    const std::string& filepath, 
    int& width, 
    int& height, 
    int& channels
)
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