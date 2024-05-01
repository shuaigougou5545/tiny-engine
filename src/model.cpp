#include "model.h"
#include <glad/glad.h>
#include <unordered_map>
#include <iostream>
#include <limits>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <filesystem>
#include "debug.h"
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

    std::cout << "[Model]: load model " << model_name << " successfully!" << std::endl;
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
    // model_name = "box";

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

    std::cout << "[Model]: load model " << model_name << " successfully!" << std::endl;
    // std::cout << "[Model]: vertices count: " << vertices.size() << std::endl;
    // std::cout << "[Model]: indices count: " << indices.size() << std::endl;
    // std::cout << "[Model]: aabb center: " << aabb.center.x << ", " << aabb.center.y <<
    //     ", " << aabb.center.z << std::endl;
    // std::cout << "[Model]: aabb extend: " << aabb.extend.x << ", " << aabb.extend.y <<
    //     ", " << aabb.extend.z << std::endl;

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

    std::cout << "[Model]: load model " << model_name << " successfully!" << std::endl;
}

void ModelLibrary::loadModels(const std::string& file_path)
{
    // collect obj files from "../resources/obj/"
    std::string path = file_path;  

    try {
        if (fs::exists(path) && fs::is_directory(path)) {
            for (const auto& entry : fs::directory_iterator(path)) {
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
}

void ModelManager::draw()
{
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, model->indices.size(), GL_UNSIGNED_INT, 0);
}

void ModelManager::reloadModel(const Model& m)
{
    if(model && m.model_name == model->model_name) {
        return;
    }
        
    
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);

    model = std::make_shared<Model>(m);

    init(); 

    // std::cout << "[Model]: reload model " << model->model_name << " successfully!" << std::endl;
}