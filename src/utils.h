#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <filesystem>


class Utils {
public:
    static bool checkDirectory(const std::string& path); // 检查目录是否存在,不存在则创建

    // 将纹理单元中的图片保存到本地
    static void saveTextureToPng(
        GLuint texture_id, 
        const std::string& filename, 
        int width, 
        int height, 
        int channels,
        bool flipped = true
    );

    static void saveTextureToJpg(
        const std::vector<unsigned char>& data,
        const std::string& filename, 
        int width,
        int height,
        int channels,
        bool flipped = true
    );

    // 将fbo中的颜色缓冲保存在本地
    static void readFrameBufferColorAttachmentToPng(
        GLuint fbo_id,
        const std::string& filename,
        int width,
        int height,
        int channels,
        bool flipped = true,
        bool multi_attchments = false, // 是否使用多个颜色附件
        int attachment_num = 0
    );

    static void readFrameBufferColorAttachmentToJpg(
        GLuint fbo_id,
        const std::string& filename,
        int width,
        int height,
        int channels,
        bool flipped = true,
        bool multi_attchments = false, // 是否使用多个颜色附件
        int attachment_num = 0
    );

    // 将fbo中的深度缓冲保存在本地
    static void readFrameBufferDepthAttachmentToPng(
        GLuint fbo_id,
        const std::string& filename,
        int width,
        int height,
        bool flipped = true
    );

    static unsigned int loadTexture(
        const std::string& filename, 
        int& width, 
        int& height, 
        int& channels
    );
    
    static unsigned int loadCubeMap(
        const std::string& filepath, 
        int& width, 
        int& height, 
        int& channels
    );

    static std::vector<glm::mat3> loadLightSHFromTxt(const std::string& filepath); // per channel
    static std::vector<glm::mat3> loadTransportSHFromTxt(const std::string& filepath); // per vertex
};





#endif // UTILS_H