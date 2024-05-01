#ifndef FBO_H
#define FBO_H

#include <glad/glad.h>
#include <vector>
#include <string>
#include <iostream>

class Fbo {
public:
    Fbo(int w, int h, const std::vector<std::string>& _color_attachment_names) : width(w), height(h), color_attachment_names(_color_attachment_names)
    {
        // fbo

        glGenFramebuffers(1, &id);
        glBindFramebuffer(GL_FRAMEBUFFER, id);

        // color

        int numColorAttachments = color_attachment_names.size();
        color_texture_ids.resize(numColorAttachments);
        glGenTextures(numColorAttachments, color_texture_ids.data());
        for(int i = 0; i < numColorAttachments; ++i) {
            glBindTexture(GL_TEXTURE_2D, color_texture_ids[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, color_texture_ids[i], 0);
        }
        glBindTexture(GL_TEXTURE_2D, 0);

        // depth

        glGenTextures(1, &depth_texture_id);
        glBindTexture(GL_TEXTURE_2D, depth_texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // 超过深度图范围的部分不受影响
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        GLfloat borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        
        glBindTexture(GL_TEXTURE_2D, 0);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture_id, 0);

        // MRT
        if(numColorAttachments > 1) {
            std::vector<GLuint> attachments(numColorAttachments);
            for(int i = 0; i < numColorAttachments; ++i) {
                attachments[i] = GL_COLOR_ATTACHMENT0 + i;
            } 
            glDrawBuffers(numColorAttachments, attachments.data());
        }
        
        checkFramebufferStatus();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    ~Fbo() 
    {
        glDeleteFramebuffers(1, &id);
        for(int i = 0; i < color_texture_ids.size(); ++i) {
            glDeleteTextures(1, &color_texture_ids[i]);
        }
        glDeleteTextures(1, &depth_texture_id);
    }

    void use()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, id);
    }

    void unuse()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void checkFramebufferStatus()
    {
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cout << "[Fbo]: Framebuffer is not complete! | ";
            
            GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            switch(status)
            {
                case GL_FRAMEBUFFER_UNDEFINED:
                    std::cout << "Framebuffer undefined" << std::endl;
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                    std::cout << "Framebuffer incomplete attachment" << std::endl;
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                    std::cout << "Framebuffer incomplete missing attachment" << std::endl;
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                    std::cout << "Framebuffer incomplete draw buffer" << std::endl;
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                    std::cout << "Framebuffer incomplete read buffer" << std::endl;
                    break;
                case GL_FRAMEBUFFER_UNSUPPORTED:
                    std::cout << "Framebuffer unsupported" << std::endl;
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                    std::cout << "Framebuffer incomplete multisample" << std::endl;
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
                    std::cout << "Framebuffer incomplete layer targets" << std::endl;
                    break;
                default:
                    std::cout << "Framebuffer unknown error" << std::endl;
                    break;
            }
        } else {
            std::cout << "[Fbo]: Create FBO successfully!" << std::endl;
        }
    }

    int getColorAttachmentsNum()
    {
        return color_texture_ids.size();
    }

    std::string getColorAttachmentsName(int i)
    {
        return color_attachment_names[i];
    }

    unsigned int id;
    std::vector<unsigned int> color_texture_ids;
    std::vector<std::string> color_attachment_names;
    unsigned int depth_texture_id; // depth stencil
    int width, height;
};

/*
    FBO常见问题:
    1. FBO需要单独清理缓冲区 - 调用glClear
*/
#endif // FBO_H