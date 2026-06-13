#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "tgaimage.h"

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

struct Vertex {
    int index;

    float x;
    float y;
    float z;
};

struct Triangle {
    Vertex p1;
    Vertex p2;
    Vertex p3;
};

void line(int ax, int ay, int bx, int by, TGAImage &framebuffer, TGAColor color);
void triangle(int ax, int ay, int bx, int by, int cx, int cy, TGAImage &framebuffer, TGAColor color);
std::tuple<int, int> project_vert(Vertex vert, int width, int height);
void render_wireframe(std::ifstream &file, TGAImage &framebuffer, int width, int height);

int main(int argc, char** argv) {
    constexpr int width  = 128;
    constexpr int height = 128;

    TGAImage framebuffer(width, height, TGAImage::RGB);
    // std::ifstream file("./obj/diablo3_pose/diablo3_pose.obj");
    // render_wireframe(file, framebuffer, width, height);

    triangle(  7, 45, 35, 100, 45,  60, framebuffer, red);
    triangle(120, 35, 90,   5, 45, 110, framebuffer, white);
    triangle(115, 83, 80,  90, 85, 120, framebuffer, green);

    framebuffer.write_tga_file("./framebuffer.tga", true, false);
    return 0;
}

void line(int ax, int ay, int bx, int by, TGAImage &framebuffer, TGAColor color) {
    bool steep = std::abs(ax - bx) < std::abs(ay - by);

    if (steep) {
        std::swap(ax, ay);
        std::swap(bx, by);
    }

    if (ax > bx) {
        std::swap(ax, bx);
        std::swap(ay, by);
    }

    float y = ay;
    int ierror = 0;

    for (int x = ax; x <= bx; x++) {
        if (steep) {
            framebuffer.set(y, x, color);
        } else {
            framebuffer.set(x, y, color);
        }
        
        ierror += 2 * std::abs(ay - by);
        y += (by > ay ? 1 : -1) * (ierror > bx - ax);
        ierror -= 2 * (bx - ax) * (ierror > bx - ax);
    }
}

void triangle(int ax, int ay, int bx, int by, int cx, int cy, TGAImage &framebuffer, TGAColor color) {
    line(ax, ay, bx, by, framebuffer, color);
    line(bx, by, cx, cy, framebuffer, color);
    line(cx, cy, ax, ay, framebuffer, color);
}

std::tuple<int, int> project_vert(Vertex vert, int width, int height) {
    int x = static_cast<int>((vert.x + 1.0)* width  / 2.0);
    int y = static_cast<int>((vert.y + 1.0)* height / 2.0);
    return {x, y};
}

void render_wireframe(std::ifstream &file, TGAImage &framebuffer, int width, int height) {
    std::vector<Vertex> vertices;
    std::vector<Triangle> triangles;

    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream lstream(line);
            std::string _;

            if (line.starts_with("v ")) {
                Vertex vert;
                
                lstream >> _ >> vert.x >> vert.y >> vert.z;
                vertices.push_back(vert);
            }

            if (line.starts_with("f ")) {
                Triangle tri;
                
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

    for (Triangle const triangle : triangles) {
        auto [t1x, t1y] = project_vert(triangle.p1, width, height);
        auto [t2x, t2y] = project_vert(triangle.p2, width, height);
        auto [t3x, t3y] = project_vert(triangle.p3, width, height);

        line(t1x, t1y, t2x, t2y, framebuffer, red);
        line(t1x, t1y, t3x, t3y, framebuffer, red);
        line(t3x, t3y, t2x, t2y, framebuffer, red);
    }
}