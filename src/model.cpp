#include "model.h"
#include <glad/glad.h>
#include <unordered_map>
#include <iostream>
#include <limits>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <filesystem>
#include "debug.h"
#include "utils.h"
namespace fs = std::filesystem;

void Model::normalize()
{
    // float max_length = std::max(aabb.extend.x, std::max(aabb.extend.y, aabb.extend.z));
    // float scale = 1.0f / max_length;
    float scale = 1.0f / aabb.extend.y; // y:[-1,1]

    for (auto& vertex : vertices)
    {
        vertex.pos.x = (vertex.pos.x - aabb.center.x) * scale;
        vertex.pos.y = (vertex.pos.y - aabb.center.y) * scale;
        vertex.pos.z = (vertex.pos.z - aabb.center.z) * scale;
    }

    // std::cout << "[Model]: normalize model " << model_name << " successfully!" << std::endl;
}


void ModelOBJ::load(std::string file_name)
{
    // model_name = file_name;

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

            if(!attrib.vertices.empty()) {
                vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };
            }

            if(!attrib.texcoords.empty()) {
                vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };
            }
            
            // 顶点法线
            if(!attrib.normals.empty()) {
                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };
            }

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

    // std::cout << "[Model]: load model " << model_name << " successfully!" << std::endl;
    // std::cout << "[Model]: vertices count: " << vertices.size() << std::endl;
    // std::cout << "[Model]: indices count: " << indices.size() << std::endl;
    // std::cout << "[Model]: aabb center: " << aabb.center.x << ", " << aabb.center.y <<
    //     ", " << aabb.center.z << std::endl;
    // std::cout << "[Model]: aabb extend: " << aabb.extend.x << ", " << aabb.extend.y <<
    //     ", " << aabb.extend.z << std::endl;

    normalize();
}

void ModelBox::load(float width, float height, float depth)
{
    vertices.clear();
    indices.clear();

    /*
        5-------6
       /|      /|
      1-------2 |
      | |     | |
      | 4-----|-7
      |/      |/
      0-------3
    */

   	Vertex v[24];

	float w2 = 0.5f * width;
	float h2 = 0.5f * height;
	float d2 = 0.5f * depth;
    
	// Fill in the front face vertex data.
	v[0] = Vertex(-w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[1] = Vertex(-w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[2] = Vertex(+w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	v[3] = Vertex(+w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	// Fill in the back face vertex data.
	v[4] = Vertex(-w2, -h2, +d2, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	v[5] = Vertex(+w2, -h2, +d2, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	v[6] = Vertex(+w2, +h2, +d2, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
	v[7] = Vertex(-w2, +h2, +d2, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);

	// Fill in the top face vertex data.
	v[8]  = Vertex(-w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
	v[9]  = Vertex(-w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
	v[10] = Vertex(+w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f);
	v[11] = Vertex(+w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f);

	// Fill in the bottom face vertex data.
	v[12] = Vertex(-w2, -h2, -d2, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f);
	v[13] = Vertex(+w2, -h2, -d2, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f);
	v[14] = Vertex(+w2, -h2, +d2, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f);
	v[15] = Vertex(-w2, -h2, +d2, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f);

	// Fill in the left face vertex data.
	v[16] = Vertex(-w2, -h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[17] = Vertex(-w2, +h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[18] = Vertex(-w2, +h2, -d2, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[19] = Vertex(-w2, -h2, -d2, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	// Fill in the right face vertex data.
	v[20] = Vertex(+w2, -h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[21] = Vertex(+w2, +h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[22] = Vertex(+w2, +h2, +d2, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[23] = Vertex(+w2, -h2, +d2, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

    vertices.assign(&v[0], &v[24]);
 
	//
	// Create the indices.
	//

	uint32_t i[36];

	// Fill in the front face index data
	i[0] = 0; i[1] = 1; i[2] = 2;
	i[3] = 0; i[4] = 2; i[5] = 3;

	// Fill in the back face index data
	i[6] = 4; i[7]  = 5; i[8]  = 6;
	i[9] = 4; i[10] = 6; i[11] = 7;

	// Fill in the top face index data
	i[12] = 8; i[13] =  9; i[14] = 10;
	i[15] = 8; i[16] = 10; i[17] = 11;

	// Fill in the bottom face index data
	i[18] = 12; i[19] = 13; i[20] = 14;
	i[21] = 12; i[22] = 14; i[23] = 15;

	// Fill in the left face index data
	i[24] = 16; i[25] = 17; i[26] = 18;
	i[27] = 16; i[28] = 18; i[29] = 19;

	// Fill in the right face index data
	i[30] = 20; i[31] = 21; i[32] = 22;
	i[33] = 20; i[34] = 22; i[35] = 23;

	indices.assign(&i[0], &i[36]);

    aabb.min = glm::vec3(-w2, -h2, -d2);
    aabb.max = glm::vec3(+w2, +h2, +d2);
    aabb.center = (aabb.max + aabb.min) / 2.0f;
    aabb.extend = (aabb.max - aabb.min) / 2.0f;
}

void ModelSphere::load(float radius)
{
    // lat: latitude 纬度
    // long: longtitude 经度
    vertices.clear();
    indices.clear();

    //
    // Create the vertices.
    //

    std::vector<Vertex> v((lat_div + 1) * (long_div + 1));
    float pi = glm::pi<float>();

     for (int lat = 0; lat <= lat_div; ++lat) {
        float theta = lat * pi / lat_div;
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);

        for (int lon = 0; lon <= long_div; ++lon) {
            float phi = lon * 2 * pi / long_div;
            float sinPhi = sin(phi);
            float cosPhi = cos(phi);

            glm::vec3 pos;
            pos.x = cosPhi * sinTheta;
            pos.y = cosTheta;
            pos.z = sinPhi * sinTheta;
            pos *= radius;

            glm::vec3 normal;
            normal.x = pos.x;
            normal.y = pos.y;
            normal.z = pos.z;
            normal = glm::normalize(normal);

            glm::vec2 texCoord;
            texCoord.x = 1.0f - (float)lon / long_div;
            texCoord.y = 1.0f - (float)lat / lat_div;

            Vertex vert;
            vert.pos = pos;
            vert.normal = normal;
            vert.texCoord = texCoord;

            v[lat * (long_div + 1) + lon] = vert;
        }
    }

    vertices.assign(&v[0], &v[(lat_div + 1) * (long_div + 1)]);

	//
	// Create the indices.
	//

    std::vector<uint32_t> i(lat_div * long_div * 6);
    uint32_t index = 0;
    for (uint32_t lat = 0; lat < lat_div; ++lat) {
        for (uint32_t lon = 0; lon < long_div; ++lon) {
            uint32_t first = (lat * (long_div + 1)) + lon;
            uint32_t second = first + long_div + 1;

            i[index++] = first;
            i[index++] = second;
            i[index++] = first + 1;

            i[index++] = second;
            i[index++] = second + 1;
            i[index++] = first + 1;
        }
    }

    indices.assign(&i[0], &i[lat_div * long_div * 6]);

    aabb.min = glm::vec3(-radius, -radius, -radius);
    aabb.max = glm::vec3(radius, radius, radius);
    aabb.center = glm::vec3(0.0f, 0.0f, 0.0f);
    aabb.extend = glm::vec3(radius, radius, radius);

    // std::cout << "[Model]: load model " << model_name << " successfully!" << std::endl;

    // normalize();
}

void ModelQuad::load()
{
    // model_name = "quad";

    vertices.clear();
    indices.clear();

    /*
        1---2
        |   |
        0---3
    */

   	Vertex v[4];

    v[0] = Vertex(-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
	v[1] = Vertex(-1.0f, +1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);  
    v[2] = Vertex(+1.0f, +1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);  
    v[3] = Vertex(+1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);  

    vertices.assign(&v[0], &v[4]);
 
	uint32_t i[6];

	i[0] = 0; i[1] = 1; i[2] = 2;
	i[3] = 0; i[4] = 2; i[5] = 3;

	indices.assign(&i[0], &i[6]);

    aabb.min = glm::vec3(-1.0f, -1.0f, 0.0f);
    aabb.max = glm::vec3(+1.0f, +1.0f, 0.0f);
    aabb.center = (aabb.max + aabb.min) / 2.0f;
    aabb.extend = (aabb.max - aabb.min) / 2.0f;

    // std::cout << "[Model]: load model " << model_name << " successfully!" << std::endl;
}

void ModelLibrary::loadModels(const std::string& file_path)
{
    // collect obj files from "../resources/obj/"
    std::string path = file_path;  

    try {
        if (fs::exists(path) && fs::is_directory(path)) {
            for (const auto& entry : fs::directory_iterator(path)) {
                // 子目录下的obj,不包括not-included
                if(entry.is_directory() && entry.path().stem().string() != "not-included") {
                    for(const auto& sub_entry : fs::directory_iterator(entry.path())) {
                        if(sub_entry.path().extension() == ".obj") {
                            std::string name = sub_entry.path().stem().string();
                            std::string filename = sub_entry.path().string();
                            std::shared_ptr<ModelOBJ> model_ptr = std::make_shared<ModelOBJ>(filename);
                            model_ptr->model_name = name;
                            model_dict.emplace(name, model_ptr);
                            
                            model_names.push_back(name);
                        }
                    }
                }

                // obj
                if (entry.path().extension() == ".obj") {
                    // .path(): 完整路径; 
                    // .path().filename(): 纯文件+后缀; 
                    // .path().stem(): 纯文件+无后缀;
                    std::string name = entry.path().stem().string();
                    std::string filename = entry.path().string();
                    model_dict.emplace(name, std::make_shared<ModelOBJ>(filename));
                    model_names.push_back(name);
                }
            }
        }
    } catch (fs::filesystem_error& e) {
        std::cerr << e.what() << std::endl;
    }
}

ModelManager::~ModelManager()
{
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
}

void ModelManager::init()
{
    if(!model)
    {
        std::cout << "[ModelManager]: model is empty!" << std::endl;
        return;
    }
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ARRAY_BUFFER, model->vertices.size() * sizeof(Vertex), model->vertices.data(), GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model->indices.size() * sizeof(unsigned int), model->indices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    use_spherical_harmonics = false;
}

void ModelManager::init(const std::string& filename)
{
    if(!model)
    {
        std::cout << "[ModelManager]: model is empty!" << std::endl;
        return;
    }

    auto LT = Utils::loadTransportSHFromTxt(filename);
    auto total_size = model->vertices.size() * sizeof(Vertex) + LT.size() * sizeof(glm::mat3);

    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ARRAY_BUFFER, total_size, nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, model->vertices.size() * sizeof(Vertex), model->vertices.data());
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model->indices.size() * sizeof(unsigned int), model->indices.data(), GL_STATIC_DRAW);
    // Light Transport
    auto offset = model->vertices.size() * sizeof(Vertex);
    glBufferSubData(GL_ARRAY_BUFFER, offset, LT.size() * sizeof(glm::mat3), LT.data());
    glCheckError();
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    int baseIndex = 3;
    int stride = sizeof(glm::mat3);
    for(int i = 0; i < 3; ++i)
    {
        glVertexAttribPointer(baseIndex + i, 3, GL_FLOAT, GL_FALSE, stride, (void*)(offset + i * sizeof(glm::vec3)));
        glEnableVertexAttribArray(baseIndex + i);
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    std::cout << "[ModelManager]: init model " << model->model_name << " with SH successfully!" << std::endl;
    use_spherical_harmonics = true;
}

void ModelManager::draw()
{
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, model->indices.size(), GL_UNSIGNED_INT, 0);
}

void ModelManager::reloadModel(const Model& m)
{
    if(model && m.model_name == model->model_name && use_spherical_harmonics == false) {
        return;
    }
    
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);

    model = std::make_shared<Model>(m);

    init(); 
    use_spherical_harmonics = false;
    // std::cout << "[Model]: reload model " << model->model_name << " successfully!" << std::endl;
}

void ModelManager::reloadModel(const Model& m, const std::string& transport_sh_filename)
{
    if(model && m.model_name == model->model_name && use_spherical_harmonics == true) {
        return;
    }
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
    model = std::make_shared<Model>(m);
    init(transport_sh_filename); 
    use_spherical_harmonics = true;
}