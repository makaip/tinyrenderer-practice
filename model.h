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
        int norm_iter = 0;

        while (std::getline(file, line)) {
            std::istringstream lstream(line);
            std::string _;

            if (line.starts_with("v ")) {
                Point3D point;
                lstream >> _ >> point.pos.x >> point.pos.y >>
                    point.pos.z;
                vertices.push_back(point);
            } else if (line.starts_with("vn ")) {
                if (norm_iter < vertices.size()) {
                    Point3D& vert = vertices.at(norm_iter);
                    lstream >> _ >> vert.norm.x >> vert.norm.y >> vert.norm.z;
                    ++norm_iter;
                }
            } else if (line.starts_with("f ")) {
                std::string v1, v2, v3;
                lstream >> _ >> v1 >> v2 >> v3;

                auto parse_index = [](const std::string& s) {
                    return std::stoi(s.substr(0, s.find('/')));
                };

                Triangle3D tri{
                    static_cast<size_t>(parse_index(v1) - 1),
                    static_cast<size_t>(parse_index(v2) - 1),
                    static_cast<size_t>(parse_index(v3) - 1),
                };

                triangles.push_back(tri);
            } else {
                continue;
            }
        }
    }
};