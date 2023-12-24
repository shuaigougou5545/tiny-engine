#include "model_object.h"
#include <unordered_map>
#include <iostream>
#include <limits>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>


void ModelObject::load(std::string file_name)
{
    model_name = file_name;

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, file_name.c_str())) {
        throw std::runtime_error(warn + err);
    }

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    vertices.clear();
    indices.clear();

    aabb.min = { std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
    aabb.max = { std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min() };

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};

            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };

            vertex.normal = {
                attrib.normals[3 * index.normal_index + 0],
                attrib.normals[3 * index.normal_index + 1],
                attrib.normals[3 * index.normal_index + 2]
            };

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);

                // calculate aabb
                aabb.min.x = std::min(aabb.min.x, vertex.pos.x);
                aabb.min.y = std::min(aabb.min.y, vertex.pos.y);
                aabb.min.z = std::min(aabb.min.z, vertex.pos.z);

                aabb.max.x = std::max(aabb.max.x, vertex.pos.x);
                aabb.max.y = std::max(aabb.max.y, vertex.pos.y);
                aabb.max.z = std::max(aabb.max.z, vertex.pos.z);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }

    aabb.center = (aabb.max + aabb.min) / 2.0f;
    aabb.extend = (aabb.max - aabb.min) / 2.0f;

    std::cout << "[Model]: load model " << file_name << " successfully!" << std::endl;
    std::cout << "[Model]: vertices count: " << vertices.size() << std::endl;
    std::cout << "[Model]: indices count: " << indices.size() << std::endl;
    std::cout << "[Model]: aabb center: " << aabb.center.x << ", " << aabb.center.y <<
        ", " << aabb.center.z << std::endl;
    std::cout << "[Model]: aabb extend: " << aabb.extend.x << ", " << aabb.extend.y <<
        ", " << aabb.extend.z << std::endl;

    normalize();
}

void ModelObject::normalize()
{
    float max_length = std::max(aabb.extend.x, std::max(aabb.extend.y, aabb.extend.z));
    float scale = 1.0f / max_length;

    for (auto& vertex : vertices)
    {
        vertex.pos.x = (vertex.pos.x - aabb.center.x) * scale;
        vertex.pos.y = (vertex.pos.y - aabb.center.y) * scale;
        vertex.pos.z = (vertex.pos.z - aabb.center.z) * scale;
    }

    std::cout << "[Model]: normalize model " << model_name << " successfully!" << std::endl;
}