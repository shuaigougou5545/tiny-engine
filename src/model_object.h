#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 texCoord;

    bool operator==(const Vertex& other) const {
        return pos == other.pos &&
               normal == other.normal &&
               texCoord == other.texCoord;
    }
};

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^
                (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
                (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}

struct AABB {
    glm::vec3 min; 
    glm::vec3 max;
    glm::vec3 center;
    glm::vec3 extend; // 半长轴
};

class ModelObject {
public:
    void load(std::string file_name);

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    std::string model_name;

    AABB aabb;

    void normalize(); // move to center & normalize position
};