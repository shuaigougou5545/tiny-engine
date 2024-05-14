#include "harmonics.h"
#include <ctime>
#include <fstream>
#include <filesystem>
#include <random>
#include "stb_image.h"
#include "stb_image_write.h"
#include "utils.h"

double Harmonics::SHEval(int l, int m, const glm::dvec3& dir)
{
    double coeff = 0.0;
    double PI = glm::pi<double>();
    glm::dvec3 dir_norm = glm::normalize(dir);
    const double x = dir_norm.x;
    const double y = dir_norm.y;
    const double z = dir_norm.z;
    
    if(l == 0) {
        if(m == 0) {
             // 0.5 * sqrt(1/pi)
            coeff = 0.282095;
        }
    }
    else if(l == 1) {
        if(m == -1) {
             // -sqrt(3/(4pi)) * y
            coeff = -0.488603 * y;
        }else if(m == 0) {
            // sqrt(3/(4pi)) * z
            coeff = 0.488603 * z;
        }else if(m == 1) {
            // -sqrt(3/(4pi)) * x
            coeff = -0.488603 * x;
        }
    }
    else if(l == 2) {
        if(m == -2) {
            // 0.5 * sqrt(15/pi) * x * y
            coeff = 1.092548 * x * y;
        } else if(m == -1) {
            // -0.5 * sqrt(15/pi) * y * z
            coeff = -1.092548 * y * z;
        } else if(m == 0) {
            // 0.25 * sqrt(5/pi) * (-x^2-y^2+2z^2)
            coeff = 0.315392 * (-x * x - y * y + 2 * z * z);
        } else if(m == 1) {
            // -0.5 * sqrt(15/pi) * x * z
            coeff = -1.092548 * x * z;
        } else if(m == 2) {
            // 0.25 * sqrt(15/pi) * (x^2 - y^2)
            coeff = 0.546274 * (x * x - y * y);
        }
    }
    else if(l == 3) {
        if(m == -3) {
            // -0.25 * sqrt(35/(2pi)) * y * (3x^2 - y^2)
            coeff = -0.590044 * y * (3.0 * x * x - y * y);
        } else if(m == -2) {
            // 0.5 * sqrt(105/pi) * x * y * z
            coeff = 2.890611 * x * y * z;
        } else if(m == -1) {
            // -0.25 * sqrt(21/(2pi)) * y * (4z^2-x^2-y^2)
            coeff = -0.457046 * y * (4.0 * z * z - x * x - y * y);
        } else if(m == 0) {
            // 0.25 * sqrt(7/pi) * z * (2z^2 - 3x^2 - 3y^2)
            coeff = 0.373176 * z * (2.0 * z * z - 3.0 * x * x - 3.0 * y * y);
        } else if(m == 1) {
            // -0.25 * sqrt(21/(2pi)) * x * (4z^2-x^2-y^2)
            coeff = -0.457046 * x * (4.0 * z * z - x * x - y * y);
        } else if(m == 2) {
            // 0.25 * sqrt(105/pi) * z * (x^2 - y^2)
            coeff = 1.445306 * z * (x * x - y * y);
        } else if(m == 3) {
            // -0.25 * sqrt(35/(2pi)) * x * (x^2-3y^2)
            coeff =  -0.590044 * x * (x * x - 3.0 * y * y);
        }
    }
    
	return coeff;
}

double Harmonics::SHEval(int l, int m, double phi, double theta)
{
    return SHEval(l, m, toVector(phi, theta));
}

std::unique_ptr<std::vector<double>> Harmonics::projectFunction(const SphericalFunction& func, int sample_count)
{
    double PI = glm::pi<double>();
    const int sample_side = static_cast<int>(floor(sqrt(sample_count)));
    std::unique_ptr<std::vector<double>> coeffs(new std::vector<double>(SHNum, 0.0));
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> rng(0.0, 1.0);

    for (int t = 0; t < sample_side; t++) 
    {
        for (int p = 0; p < sample_side; p++) 
        {
            double alpha = (t + rng(gen)) / sample_side;
            double beta = (p + rng(gen)) / sample_side;
            // See http://www.bogotobogo.com/Algorithms/uniform_distribution_sphere.php
            double phi = 2.0 * M_PI * beta;
            double theta = acos(2.0 * alpha - 1.0);

            double func_value = func(phi, theta);

            for (int l = 0; l <= SHOrder; ++l) 
            {
                for (int m = -l; m <= l; ++m) 
                {
                    double sh = SHEval(l, m, phi, theta);
                    (*coeffs)[getIndex(l, m)] += func_value * sh;
                }
            }
        }
    }

    double weight = 4.0 * PI / (sample_side * sample_side); // 表面积 / 采样个数
    for (unsigned int i = 0; i < coeffs->size(); ++i) 
    {
        (*coeffs)[i] *= weight;
    }

    return coeffs;
}

float Harmonics::calcPreArea(const float& x, const float& y)
{
    return std::atan2(x * y, std::sqrt(x * x + y * y + 1.0));
}

float Harmonics::calcArea(const float& u_, const float& v_, const int &width, const int& height)
{
    // transform from [0..res - 1] to [- (1 - 1 / res) .. (1 - 1 / res)]
    // ( 0.5 is for texel center addressing)
    float u = (2.0 * (u_ + 0.5) / width) - 1.0;
    float v = (2.0 * (v_ + 0.5) / height) - 1.0;

    // shift from a demi texel, mean 1.0 / size  with u and v in [-1..1]
    float invResolutionW = 1.0 / width;
    float invResolutionH = 1.0 / height;

    // u and v are the -1..1 texture coordinate on the current face.
    // get projected area for this texel
    float x0 = u - invResolutionW;
    float y0 = v - invResolutionH;
    float x1 = u + invResolutionW;
    float y1 = v + invResolutionH;
    float angle = calcPreArea(x0, y0) - calcPreArea(x0, y1) -
                    calcPreArea(x1, y0) + calcPreArea(x1, y1);

    return angle;
}

std::vector<glm::vec3> Harmonics::computeCubemapSH(const std::string& filepath)
{
    std::vector<glm::vec3> coeffs(SHNum, glm::vec3(0.0)); // rgb

    std::cout << "[Harmonics]:  Calculating Cubemap to SH..." << std::endl;
    clock_t start_time = clock();

    // load cubemap
    std::vector<std::string> suffixes = { "posx", "negx", "posy", "negy", "posz", "negz" };
    std::string extension;

    // supported: .jpg .png
    if(std::filesystem::exists(filepath + "/" + suffixes[0] + ".jpg")) {
        extension = ".jpg";
    } else if(std::filesystem::exists(filepath + "/" + suffixes[0] + ".png")) {
        extension = ".png";
    }

    std::vector<std::string> textures_faces;
    for(auto& suffix : suffixes)
        textures_faces.push_back(filepath + "/" + suffix + extension);
    
    
    // save cubemap data
    std::vector<std::unique_ptr<float[]>> images(6); // save cubemap data
    float *image;  
    int width, height, channels;
    for(unsigned int i = 0; i < textures_faces.size(); i++)
    {
        int w, h, c;
        image = stbi_loadf(textures_faces[i].c_str(), &w, &h, &c, 3);

        if(!image)
        {
            std::cout << "[Harmonics]: load cubemap texture " << textures_faces[i] << " failed!" << std::endl;
            exit(-1);
        }

        if(i == 0)
        {
            width = w;
            height = h;
            channels = c;
        }

        images[i] = std::unique_ptr<float[]>(image);
    }

    //
    // cubemap dir
    std::vector<glm::vec3> cubemapDirs;
    cubemapDirs.reserve(6 * width * height);
    for(int i = 0; i < 6; ++i)
    {
        glm::vec3 dir_x = cubemapFaceDirections[i][0];
        glm::vec3 dir_y = cubemapFaceDirections[i][1];
        glm::vec3 dir_z = cubemapFaceDirections[i][2];
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                float u = 2 * ((x + 0.5) / width) - 1;
                float v = 2 * ((y + 0.5) / height) - 1;
                glm::dvec3 dir = glm::normalize(dir_x * u + dir_y * v + dir_z);
                cubemapDirs.push_back(dir);
            }
        }
    }

    //
    // calc SH
    for(int i = 0; i < 6; ++i) 
    {
        // for each face
        for(int y = 0; y < height; ++y)
        {
            for(int x = 0; x < width; ++x)
            {
                // for each pixel on cubemap
                glm::dvec3 dir = cubemapDirs[i * width * height + y * width + x];
                int index = (y * width + x) * channels;
                glm::vec3 Le(images[i][index], images[i][index + 1], images[i][index + 2]);

                auto dwi = calcArea(x, y, width, height);
                for(int l = 0; l <= SHOrder; ++l)
                {
                    for(int m = -l; m <= l; ++m)
                    {
                        float sh = static_cast<float>(SHEval(l, m, glm::normalize(dir)));
                        coeffs[getIndex(l, m)] += Le * sh * dwi;
                    }
                }
            }
        }
    }

    // for(const auto& coeff : coeffs) 
    //     std::cout << coeff.x << " " << coeff.y << " " << coeff.z << std::endl;
    // std::cout << "------------------------" << std::endl;
    clock_t end_time = clock();
    double elapsed_time = (end_time - start_time) / (double)CLOCKS_PER_SEC;
    std::cout << "[Harmonics]:  Time taken " << elapsed_time << " seconds." << std::endl;

    // write into light.txt
    std::string out_file_name = filepath + "/light.txt";
    std::ofstream outFile(out_file_name);
    for(const auto& coeff : coeffs)
    {
        outFile << coeff.x << " " << coeff.y << " " << coeff.z << std::endl;
    }
    outFile.close();
    std::cout << "[Harmonics]:  Light coefficients written into " << out_file_name << std::endl;

    return coeffs;
}

std::vector<glm::vec3> Harmonics::computeCubemapSH(const std::vector<std::vector<unsigned char>>& images, int width, int height, int channels, const std::string& outfile_path)
{
    std::vector<glm::vec3> coeffs(SHNum, glm::vec3(0.0)); // rgb

    std::cout << "[Harmonics]:  Calculating Cubemap to SH..." << std::endl;
    clock_t start_time = clock();

    //
    // cubemap dir
    std::vector<glm::vec3> cubemapDirs;
    cubemapDirs.reserve(6 * width * height);
    for(int i = 0; i < 6; ++i)
    {
        glm::vec3 dir_x = cubemapFaceDirections[i][0];
        glm::vec3 dir_y = cubemapFaceDirections[i][1];
        glm::vec3 dir_z = cubemapFaceDirections[i][2];
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                float u = 2 * ((x + 0.5) / width) - 1;
                float v = 2 * ((y + 0.5) / height) - 1;
                glm::dvec3 dir = glm::normalize(dir_x * u + dir_y * v + dir_z);
                cubemapDirs.push_back(dir);
            }
        }
    }

    //
    // calc SH
    for(int i = 0; i < 6; ++i) 
    {
        // for each face
        for(int y = 0; y < height; ++y)
        {
            for(int x = 0; x < width; ++x)
            {
                // for each pixel on cubemap
                glm::dvec3 dir = cubemapDirs[i * width * height + y * width + x];
                int index = (y * width + x) * channels;
                glm::vec3 Le(images[i][index] / 255.f, images[i][index + 1] / 255.f, images[i][index + 2] / 255.f); // unsigned char -> float

                auto dwi = calcArea(x, y, width, height);
                for(int l = 0; l <= SHOrder; ++l)
                {
                    for(int m = -l; m <= l; ++m)
                    {
                        float sh = static_cast<float>(SHEval(l, m, glm::normalize(dir)));
                        coeffs[getIndex(l, m)] += Le * sh * dwi;
                    }
                }
            }
        }
    }

    clock_t end_time = clock();
    double elapsed_time = (end_time - start_time) / (double)CLOCKS_PER_SEC;
    std::cout << "[Harmonics]:  Time taken " << elapsed_time << " seconds." << std::endl;

    // write into light.txt
    std::string out_file_name = outfile_path; // outfile
    std::ofstream outFile(out_file_name);
    for(const auto& coeff : coeffs)
    {
        outFile << coeff.x << " " << coeff.y << " " << coeff.z << std::endl;
    }
    outFile.close();
    std::cout << "[Harmonics]:  Light coefficients written into " << out_file_name << std::endl;

    return coeffs;
}

std::vector<glm::mat3> Harmonics::convertSH(const std::vector<glm::vec3>& sh)
{
    // std::vector<glm::vec3> => 每个元素为rgb
    // std::vector<glm::mat3> => 每个通道集中在一个mat3

    std::vector<glm::mat3> coeffs(3, glm::mat3(0.f));
    for(int i = 0; i < sh.size(); ++i)
    {
        for(int j = 0; j < 3; ++j)
        {
            coeffs[j][i / 3][i % 3] = sh[i][j];
        }
    }
    return coeffs;
}

std::vector<std::vector<float>> Harmonics::computeVerticesSH(const Model& model)
{
    const int vertices_count = model.vertices.size();
    std::vector<std::vector<float>> coeffs(vertices_count, std::vector<float>(SHNum, 0.0));

    std::cout << "[Harmonics]:  Calculating Vertices to SH..." << std::endl;
    clock_t start_time = clock();

    double PI = glm::pi<double>();
    

    const int sample_count = 100;
    auto& vertices = model.vertices;
    for(int i = 0; i < vertices_count; ++i)
    {
        const glm::vec3 pos = vertices[i].pos;
        const glm::vec3 normal = glm::normalize(vertices[i].normal);

        auto shFunc = [&](double phi, double theta) -> double {
            glm::vec3 dir = glm::normalize(toVector(phi, theta));
            double H = glm::dot(dir, normal);
            return std::max(H, 0.0); // unshadowed + diffuse
        };

        auto coeffs_ptr = projectFunction(shFunc, sample_count);
        for(int j = 0; j < SHNum; ++j)
        {
            coeffs[i][j] += (*coeffs_ptr)[j] / PI; // (÷𝜋) : diffuse brdf = albedo/𝜋
        }
    }

    std::cout << "[Harmonics]:  Calculated " << vertices_count << " Vertices to SH!" << std::endl;
    clock_t end_time = clock();
    double elapsed_time = (end_time - start_time) / (double)CLOCKS_PER_SEC;
    std::cout << "[Harmonics]:  Time taken " << elapsed_time << " seconds." << std::endl;

    // write into transport.txt
    std::string out_file_name = "../resources/obj/" + model.model_name + "/transport.txt"; // ../resources/obj/bunny/transport.txt
    Utils::checkDirectory(out_file_name);
    
    std::ofstream outFile(out_file_name);
    outFile << coeffs.size() << std::endl;
    for(int i = 0; i < coeffs.size(); ++i)
    {
        for(int j = 0; j < coeffs[i].size(); ++j)
        {
            outFile << coeffs[i][j] << " ";
        }
        outFile << std::endl;
    }
    outFile.close();
    std::cout << "[Harmonics]:  Transport coefficients written into " << out_file_name << std::endl;

    return coeffs;
}