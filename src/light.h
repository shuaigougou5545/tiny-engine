#ifndef LIGHT_H
#define LIGHT_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>


enum LightProjectFunc {
    ORTHOGONAL,
    PERSPECTIVE
};

class Light {
public:
    Light(const glm::vec3& pos, 
        const glm::vec3& strength, 
        LightProjectFunc project_func, 
        const glm::vec2& resolution,
        const glm::vec3& target = glm::vec3(0.0)
    ) : Position(pos), Strength(strength), ProjectionFunction(project_func)
    {
        Direction = target - pos;
        aspect_ratio = resolution.x / resolution.y;
        update();
    }

    void update()
    {
        view_matrix = glm::lookAt(Position, Position + Direction, glm::vec3(0.0, 1.0f, 0.0f));
        switch(ProjectionFunction) {
            case PERSPECTIVE:
                projection_matrix = glm::perspective(fovy, aspect_ratio, near_z, far_z);
                break;
            case ORTHOGONAL:
                projection_matrix = glm::ortho(left, right, bottom, top, near_z, far_z);
                break;
        }
    }
    
    glm::vec3 Position; // in world space
    glm::vec3 Direction; 
    glm::vec3 Strength;
    float near_z = 0.1f, far_z = 100.0f;

    // 
    // view
    glm::mat4 view_matrix;

    // 
    // projection

    // perspective
    float fovy = glm::radians(45.0f);
    float aspect_ratio = 1.0f;
    // orthogonal
    float left = -1.0;
    float right = 1.0;
    float bottom = -1.0;
    float top = 1.0;
    // project func
    LightProjectFunc ProjectionFunction;
    // project matrix
    glm::mat4 projection_matrix; // perspective or orthogonal
};

#endif // LIGHT_H
