#pragma once

#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "vecs.h"

struct Point3D {
    vec3 pos;
    vec3 norm;
};

struct Triangle3D {
    size_t p1;
    size_t p2;
    size_t p3;
};

class Model {
   public:
    std::vector<Point3D> vertices;
    std::vector<Triangle3D> triangles;

    Model(const std::string filename) {
        std::ifstream file(filename);

        if (!file.is_open()) throw std::runtime_error("File did not load");

        std::string line;
        std::vector<vec3> temp_normals;
        int norm_iter = 0;

        while (std::getline(file, line)) {
            std::istringstream lstream(line);
            std::string _;

            if (line.starts_with("v ")) {
                Point3D point;
                lstream >> _ >> point.pos.x >> point.pos.y >> point.pos.z;
                vertices.push_back(point);
            } else if (line.starts_with("vn ")) {
                vec3 norm;
                lstream >> _ >> norm.x >> norm.y >> norm.z;
                temp_normals.push_back(norm);
            } else if (line.starts_with("f ")) {
                std::string v1, v2, v3;
                lstream >> _ >> v1 >> v2 >> v3;

                auto parse_vecs = [](const std::string& s) {
                    return std::stoi(s.substr(0, s.find('/')));
                };

                auto parse_norms = [](const std::string& s) {
                    return std::stoi(s.substr(s.find_last_of('/') + 1));
                };

                size_t idx1 = parse_vecs(v1) - 1;
                size_t idx2 = parse_vecs(v2) - 1;
                size_t idx3 = parse_vecs(v3) - 1;

                if (!temp_normals.empty()) {
                    vertices[idx1].norm = temp_normals[parse_norms(v1) - 1];
                    vertices[idx2].norm = temp_normals[parse_norms(v2) - 1];
                    vertices[idx3].norm = temp_normals[parse_norms(v3) - 3];
                }

                Triangle3D tri{idx1, idx2, idx3};
                triangles.push_back(tri);
            } else {
                continue;
            }
        }
    }
};