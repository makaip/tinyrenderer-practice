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

void line(int ax, int ay, int bx, int by, TGAImage &framebuffer, TGAColor color);
void draw_triangle(int ax, int ay, int az, int bx, int by, int bz, int cx, int cy, int cz, TGAImage &framebuffer);
float tri_area(int ax, int ay, int bx, int by, int cx, int cy);
std::tuple<int, int> project_vert(Vertex3D vert, int width, int height);
void render_view(std::ifstream &file, TGAImage &framebuffer, int width, int height);

int main(int argc, char** argv) {
    constexpr int width  = 800;
    constexpr int height = 800;

    TGAImage framebuffer(width, height, TGAImage::RGB);
    // std::ifstream file("./obj/diablo3_pose/diablo3_pose.obj");
    // render_view(file, framebuffer, width, height);

    int ax = 17, ay =  4, az =  13;
    int bx = 55, by = 39, bz = 128;
    int cx = 23, cy = 59, cz = 255;

    draw_triangle(ax, ay, az, bx, by, bz, cx, cy, cz, framebuffer);

    framebuffer.write_tga_file("./framebuffer.tga", true, false);
    return 0;
}

float tri_area(int ax, int ay, int bx, int by, int cx, int cy) {
    return (ax * (by - cy) + bx * (cy - ay) + cx * (ay - by)) / 2;
}

void draw_triangle(int ax, int ay, int az, int bx, int by, int bz, int cx, int cy, int cz, TGAImage &framebuffer) {
    float area = tri_area(ax, ay, bx, by, cx, cy);

    if (area <= 0)
        return;
    
    int bbminx = std::min(std::min(ax, bx), cx);
    int bbminy = std::min(std::min(ay, by), cy);
    int bbmaxx = std::max(std::max(ax, bx), cx);
    int bbmaxy = std::max(std::max(ay, by), cy);

    # pragma omp parallel for
    for (int px = 0; px < framebuffer.width(); ++px) {
        for (int py = 0; py < framebuffer.height(); ++py) {
            double a = tri_area(px, py, bx, by, cx, cy) / area;
            double b = tri_area(px, py, cx, cy, ax, ay) / area;
            double g = tri_area(px, py, ax, ay, bx, by) / area;

            if (a < 0 || b < 0 || g < 0) continue;
            
            TGAColor z = {
                std::round(a * float(az) + b * float(bz) + g * float(cz)), 
                std::round(b * float(az) + g * float(bz) + a * float(cz)), 
                std::round(g * float(az) + a * float(bz) + b * float(cz)), 
                1
            };
            
            framebuffer.set(px, py, z);
        }
    }
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


std::tuple<int, int> project_vert(Vertex3D vert, int width, int height) {
    int x = static_cast<int>((vert.x + 1.0)* width  / 2.0);
    int y = static_cast<int>((vert.y + 1.0)* height / 2.0);
    return {x, y};
}

void render_view(std::ifstream &file, TGAImage &framebuffer, int width, int height) {
    std::vector<Vertex3D> vertices;
    std::vector<Triangle3D> triangles;

    if (file.is_open()) {
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

    for (Triangle3D const triangle : triangles) {
        auto [t1x, t1y] = project_vert(triangle.p1, width, height);
        auto [t2x, t2y] = project_vert(triangle.p2, width, height);
        auto [t3x, t3y] = project_vert(triangle.p3, width, height);

        // line(t1x, t1y, t2x, t2y, framebuffer, red);
        // line(t1x, t1y, t3x, t3y, framebuffer, red);
        // line(t3x, t3y, t2x, t2y, framebuffer, red);

        TGAColor rnd;
        for (int c=0; c<3; c++) rnd[c] = std::rand() % 255;

        // draw_triangle(t1x, t1y, t2x, t2y, t3x, t3y, framebuffer, rnd);
    }
}