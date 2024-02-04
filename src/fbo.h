#ifndef FBO_H
#define FBO_H

#include <glad/glad.h>
#include <iostream>

class Fbo {
public:
    Fbo(int w, int h, bool depth_texture = false) : width(w), height(h)
    {
        // color

        glGenTextures(1, &color_texture_id);
        glBindTexture(GL_TEXTURE_2D, color_texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        // depth

        if(depth_texture) {
            glGenTextures(1, &depth_texture);
            glBindTexture(GL_TEXTURE_2D, depth_texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glBindTexture(GL_TEXTURE_2D, 0);
        }else {
            glGenRenderbuffers(1, &depth_stencil_id);
            glBindRenderbuffer(GL_RENDERBUFFER, depth_stencil_id);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
        }

        // fbo

        glGenFramebuffers(1, &id);
        glBindFramebuffer(GL_FRAMEBUFFER, id);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_texture_id, 0);
        if(depth_texture) {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_COMPONENT16, GL_TEXTURE_2D, depth_texture, 0);
        }else {
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_stencil_id);
        }
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    ~Fbo() 
    {
        glDeleteFramebuffers(1, &id);
        glDeleteTextures(1, &color_texture_id);
        if(depth_texture) {
            glDeleteTextures(1, &depth_texture);
        }else {
            glDeleteRenderbuffers(1, &depth_stencil_id);
        }
    }

    unsigned int id;
    unsigned int color_texture_id;
    unsigned int depth_stencil_id; // depth stencil
    unsigned int depth_texture;
    int width, height;
};

#endif // FBO_H