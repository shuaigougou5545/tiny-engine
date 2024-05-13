#ifndef HARMONICS_H
#define HARMONICS_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <string>
#include <iostream>
#include "model.h"

typedef std::function<double(double, double)> SphericalFunction;

class Harmonics {
public:
    int SHOrder; // support: SHOrder <= 3
    int SHNum;

    Harmonics(int order = 2) : SHOrder(order) { SHNum = (SHOrder + 1) * (SHOrder + 1); }

    // tools
    int getIndex(int l, int m) { 
        return l * (l + 1) + m; 
    }

    glm::vec3 toVector(double phi, double theta) {
        double r = sin(theta);
        return glm::vec3(r * cos(phi), r * sin(phi), cos(theta));
    }

    // project
    double SHEval(int l, int m, const glm::dvec3& dir); 
    double SHEval(int l, int m, double phi, double theta); 
    std::unique_ptr<std::vector<double>> projectFunction(const SphericalFunction& func, int sample_count);

    // process environment map - Light
    const glm::vec3 cubemapFaceDirections[6][3] = {
        {{0, 0, 1}, {0, -1, 0}, {1, 0, 0}},   // posx
        {{0, 0, 1}, {0, -1, 0}, {-1, 0, 0}},  // negx
        {{1, 0, 0}, {0, 0, 1}, {0, 1, 0}},    // posy
        {{1, 0, 0}, {0, 0, -1}, {0, -1, 0}},  // negy
        {{1, 0, 0}, {0, -1, 0}, {0, 0, 1}},   // posz
        {{-1, 0, 0}, {0, -1, 0}, {0, 0, -1}}, // negz
    };
    // 计算Cubemap中一个像素对应的立体角大小: https://www.cnblogs.com/redips-l/p/10976173.html
    float calcPreArea(const float& x, const float& y);
    float calcArea(const float& u_, const float& v_, const int &width, const int& height);
    std::vector<glm::vec3> computeCubemapSH(const std::string& filepath);
    std::vector<glm::vec3> computeCubemapSH(const std::vector<std::vector<unsigned char>>& images, int width, int height, int channels, const std::string& outfile_path);

    std::vector<glm::mat3> convertSH(const std::vector<glm::vec3>& sh);
    // process vertices - Light Transport
    std::vector<std::vector<float>> computeVerticesSH(const Model& model);
};

#endif

/*
    Harmonics常见问题:
    1. Cube Map 的每个面可以通过展开立方体进行理解,展开立方体的过程中势必导致Top和Bottom与其他四个面不太一致
    => {注意1}一般图片的数据是从左上角到右下角的方式存储
                Top(+y)
    Left(-x)    Front(+z)   Right(+x)   Back(-z)
                Bottom(-y)
    
    => {注意1}OpenGL使用右手坐标系,即 posz = +z = Front

    所以: 可以通过这些手段检查理解是否正确 => posz与实际在屏幕上看到的是相反的,且posz的右侧是posx 
    => 可以试想以posZ(Front)为基准展开盒子 => 一个明显的特征是在渲染CubeMap时,站在中心看到的六个面,和实际渲染的是反的

    => 总结: 如果当我要去生成Cubemap的时候，要对6个面进行颠倒操作
        => Left(-x)    Front(+z)   Right(+x)   Back(-z) 进行左右颠倒，即对纹理的u方向颠倒
        => Top(+y) 和 Bottom(-y) 进行上下颠倒,即对纹理的v方向颠倒

    2.OpenGL在Shader中读取纹理，坐标从左下角开始；纹理数据往往会被解析成左上角开始，故经常会执行 stbi_flip_vertically_on_write(flipped);
*/