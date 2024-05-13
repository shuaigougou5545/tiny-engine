#ifndef LIGHT_H
#define LIGHT_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>


enum LIGHT_TYPE {
    DIRECTIONAL_LIGHT = 0,
    POINT_LIGHT = 1,
    SPOT_LIGHT = 2
};

struct OrthoFrustum {
    float l = -5.f;
    float r = +5.f;
    float b = -5.f;
    float t = +5.f;
};

struct PerspectiveFrustum {
    float fovy = glm::radians(45.f);
    float aspect_ratio = 1080.f / 720.f;
};

class Light {
public:
    LIGHT_TYPE type = POINT_LIGHT; // 0 - directional light, 1 - point light
    glm::vec3 position = glm::vec3(0.f); // point light
    glm::vec3 direction = glm::vec3(0.f, 0.f, 1.f); // directional light & point light
    glm::vec3 strength = glm::vec3(1.f);

    Light() { updateMatrix(); }

    // Light Attenuation
    float inner_cutoff = glm::radians(0.f); // min angle - spot light
    float outer_cutoff = glm::radians(20.f); // max angle - spot light
    float light_range = 100.f; // distance range - spot light
    // Frustum
    OrthoFrustum ortho_frustum;
    PerspectiveFrustum perspective_frustum;

    // Matrix
    glm::mat4 view_matrix;
    glm::mat4 projection_matrix;
    float near = 0.1f;
    float far = 100.0f;

    void updateMatrix()
    {
        switch (type)
        {
        case DIRECTIONAL_LIGHT: {
            // directional light
            // 对于平行光源来说,position位置并不重要,随意即可
            view_matrix = glm::lookAt(position, position + direction, glm::vec3(0.0, 1.0f, 0.0f));
            float l = ortho_frustum.l;
            float r = ortho_frustum.r;
            float b = ortho_frustum.b;
            float t = ortho_frustum.t;
            float n = near;
            float f = far;
            projection_matrix = glm::ortho(l, r, b, t, n, f);
            break;
        }
        case POINT_LIGHT: {
            view_matrix = glm::lookAt(position, position + direction, glm::vec3(0.0, 1.0f, 0.0f));
            float fovy = perspective_frustum.fovy;
            float aspect_ratio = perspective_frustum.aspect_ratio;
            float n = near;
            float f = far;
            projection_matrix = glm::perspective(fovy, aspect_ratio, n, f);
            break;
        }
        case SPOT_LIGHT: {
            view_matrix = glm::lookAt(position, position + direction, glm::vec3(0.0, 1.0f, 0.0f));
            float fovy = perspective_frustum.fovy;
            float aspect_ratio = perspective_frustum.aspect_ratio;
            float n = near;
            float f = far;
            projection_matrix = glm::perspective(fovy, aspect_ratio, n, f);
            break;
        }
        default:
            break;
        }
    }
};

#endif // LIGHT_H
