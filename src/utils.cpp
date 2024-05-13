#include "utils.h"

#include <vector>
#include <fstream>
#include <filesystem>
#include "stb_image.h"
#include "stb_image_write.h"
#include "debug.h"

namespace fs = std::filesystem;

bool Utils::checkDirectory(const std::string& path) 
{
    std::string directory = path.substr(0, path.find_last_of("/\\"));
    try {
        if (!fs::exists(directory)) {
            return fs::create_directories(directory);
        }
        return true; // 目录已存在
    } catch (const std::exception& e) {
        std::cerr << "Error creating directory: " << e.what() << std::endl;
        return false;
    }
}

void Utils::saveTextureToPng(
    GLuint texture_id, 
    const std::string& filename, 
    int width, 
    int height, 
    int channels,
    bool flipped
)
{
    checkDirectory(filename);
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

void Utils::saveTextureToJpg(
    const std::vector<unsigned char>& data,
    const std::string& filename, 
    int width,
    int height,
    int channels,
    bool flipped
)
{
    checkDirectory(filename);
    stbi_flip_vertically_on_write(flipped);
    stbi_write_jpg(filename.c_str(), width, height, channels, data.data(), width * channels);
}

void Utils::readFrameBufferColorAttachmentToPng(
    GLuint fbo_id,
    const std::string& filename,
    int width,
    int height,
    int channels,
    bool flipped,
    bool multi_attchments,
    int attachment_num
)
{
    checkDirectory(filename);
    std::vector<unsigned char> data;
    data.resize(width * height * channels);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
    if(multi_attchments) {
        // 单个颜色附件调用会报错
        glReadBuffer(GL_COLOR_ATTACHMENT0 + attachment_num);
    }
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    stbi_flip_vertically_on_write(flipped);
    stbi_write_png(filename.c_str(), width, height, channels, data.data(), width * channels);
    std::cout << "[Utils]: read framebuffer[id:" << fbo_id <<  "] color[id: "<< attachment_num << "] to png: " << filename << " successfully!" << std::endl;
}

void Utils::readFrameBufferColorAttachmentToJpg(
    GLuint fbo_id,
    const std::string& filename,
    int width,
    int height,
    int channels,
    bool flipped,
    bool multi_attchments, // 是否使用多个颜色附件
    int attachment_num
)
{
    checkDirectory(filename);
    std::vector<unsigned char> data;
    data.resize(width * height * channels);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
    if(multi_attchments) {
        // 单个颜色附件调用会报错
        glReadBuffer(GL_COLOR_ATTACHMENT0 + attachment_num);
    }
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    stbi_flip_vertically_on_write(flipped);
    stbi_write_jpg(filename.c_str(), width, height, channels, data.data(), width * channels);
    std::cout << "[Utils]: read framebuffer[id:" << fbo_id <<  "] color[id: "<< attachment_num << "] to png: " << filename << " successfully!" << std::endl;
}

void Utils::readFrameBufferDepthAttachmentToPng(
    GLuint fbo_id,
    const std::string& filename,
    int width,
    int height,
    bool flipped
)
{
    checkDirectory(filename);
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

    std::vector<std::string> suffixes = { "posx", "negx", "posy", "negy", "posz", "negz" };
    std::string extension;

    // supported: .jpg .png
    if(std::filesystem::exists(filepath + "/" + suffixes[0] + ".jpg")) {
        extension = ".jpg";
    } else if(std::filesystem::exists(filepath + "/" + suffixes[0] + ".png")) {
        extension = ".png";
    }

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
    glCheckError();
    return texture;
}

std::vector<glm::mat3> Utils::loadLightSHFromTxt(const std::string& filepath)
{
    std::ifstream file(filepath);

    std::vector<glm::mat3> L(3); // 注意:默认使用2阶球谐(SHOrder=2)
    for(int j = 0; j < 9; ++j)
    {
        for(int i = 0; i < 3; ++i)
        {
            int row = j / 3, col = j % 3;
            file >> L[i][row][col];
        }
    }

    file.close();
    return L;
}

std::vector<glm::mat3> Utils::loadTransportSHFromTxt(const std::string& filepath)
{
    std::ifstream file(filepath);
    int vertex_count;
    file >> vertex_count;

    std::vector<glm::mat3> LT(vertex_count); // 注意:默认使用2阶球谐(SHOrder=2)
    
    for(int i = 0; i < vertex_count; ++i)
    {
        auto& curr_mat3 = LT[i];
        file >> curr_mat3[0][0] >> curr_mat3[0][1] >> curr_mat3[0][2]
           >> curr_mat3[1][0] >> curr_mat3[1][1] >> curr_mat3[1][2]
           >> curr_mat3[2][0] >> curr_mat3[2][1] >> curr_mat3[2][2];
    }
    file.close();
    return LT;
}

