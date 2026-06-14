#pragma once

#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>


struct Vertex3D {
    int index;

    float x;
    float y;
    float z;
};

struct Triangle3D {
    Vertex3D p1;
    Vertex3D p2;
    Vertex3D p3;
};


class Model {
public:
    std::vector<Vertex3D> vertices;
    std::vector<Triangle3D> triangles;

    Model(const std::string filename) {
        std::ifstream file(filename);

        if (!file.is_open())
            throw std::runtime_error("File did not load"); 
        
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream lstream(line);
            std::string _;

            if (line.starts_with("v ")) {
                Vertex3D vert;
                
                lstream >> _ >> vert.x >> vert.y >> vert.z;
                vertices.push_back(vert);
            }

            if (line.starts_with("f ")) {
                Triangle3D tri;
                
                std::string v1, v2, v3;
                lstream >> _ >> v1 >> v2 >> v3;

                auto parse_index = [](const std::string& s) {
                    return std::stoi(s.substr(0, s.find('/')));
                };

                int p1ind = parse_index(v1);
                int p2ind = parse_index(v2);
                int p3ind = parse_index(v3);

                tri.p1 = vertices[p1ind - 1];
                tri.p2 = vertices[p2ind - 1];
                tri.p3 = vertices[p3ind - 1];
                
                triangles.push_back(tri);
            }
        }
    }
};