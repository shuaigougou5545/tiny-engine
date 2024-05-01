#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <string>
#include <glad/glad.h>


class Utils {
public:
    // 将纹理单元中的图片保存到本地
    static void saveTextureToPng(
        GLuint texture_id, 
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
};





#endif // UTILS_H