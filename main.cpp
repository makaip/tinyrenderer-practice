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
void triangle(int ax, int ay, int bx, int by, int cx, int cy, TGAImage &framebuffer, TGAColor color);
bool inside_triangle(int px, int py, int ax, int ay, int bx, int by, int cx, int cy);
std::tuple<int, int> project_vert(Vertex3D vert, int width, int height);
void render_view(std::ifstream &file, TGAImage &framebuffer, int width, int height);

int main(int argc, char** argv) {
    constexpr int width  = 800;
    constexpr int height = 800;

    TGAImage framebuffer(width, height, TGAImage::RGB);
    std::ifstream file("./obj/diablo3_pose/diablo3_pose.obj");
    render_view(file, framebuffer, width, height);

    // draw_triangle(  7, 45, 35, 100, 45,  60, framebuffer, red);
    // draw_triangle(120, 35, 90,   5, 45, 110, framebuffer, white);
    // draw_triangle(115, 83, 80,  90, 85, 120, framebuffer, green);

    framebuffer.write_tga_file("./framebuffer.tga", true, false);
    return 0;
}

bool inside_triangle(int px, int py, int ax, int ay, int bx, int by, int cx, int cy) {
    float denom = (by - cy) * (ax - cx) + (cx - bx) * (ay - cy);
    if (denom == 0)
        return false;
    
    float w1 = ((by - cy) * (px - cx) + (cx - bx) * (py - cy)) / denom;
    float w2 = ((cy - ay) * (px - cx) + (ax - cx) * (py - cy)) / denom;
    float w3 = 1 - w1 - w2;

    bool inside = w1 >= 0 && w2 >= 0 && w3 >= 0;

    return inside;
}

void draw_triangle(int ax, int ay, int bx, int by, int cx, int cy, TGAImage &framebuffer, TGAColor color) {
    float area = (ax * (by - cy) + bx * (cy - ay) + cx * (ay - by)) / 2;
    
    if (area <= 0)
        return;

    # pragma omp parallel for
    for (int px = 0; px < framebuffer.width(); ++px) {
        for (int py = 0; py < framebuffer.height(); ++py) {
            if (inside_triangle(px, py, ax, ay, bx, by, cx, cy))
                framebuffer.set(px, py, color);
        }
    }

    line(ax, ay, bx, by, framebuffer, color);
    line(bx, by, cx, cy, framebuffer, color);
    line(cx, cy, ax, ay, framebuffer, color);
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

        draw_triangle(t1x, t1y, t2x, t2y, t3x, t3y, framebuffer, rnd);
    }
}