#define _USE_MATH_DEFINES

#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "model.h"
#include "tgaimage.h"
#include "vecs.h"
#include "mats.h"


constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

void line(int ax, int ay, int bx, int by, TGAImage& framebuffer,
          TGAColor color);
void draw_triangle(int ax, int ay, int az, int bx, int by, int bz, int cx,
                   int cy, int cz, TGAImage& zbuffer, TGAImage& framebuffer,
                   TGAColor color);
float tri_area(int ax, int ay, int bx, int by, int cx, int cy);
std::tuple<int, int, int> project_vert(vec3 point, int width, int height);
void render_view(Model model, TGAImage& zbuffer, TGAImage& framebuffer,
                 int width, int height);

vec3 rot(vec3 v) {
    const double a = M_PI / 2;
    mat<3> Ry;
    Ry[0][0] =  std::cos(a); Ry[0][1] = 0; Ry[0][2] = std::sin(a);
    Ry[1][0] =  0;           Ry[1][1] = 1; Ry[1][2] = 0;
    Ry[2][0] = -std::sin(a); Ry[2][1] = 0; Ry[2][2] = std::cos(a);
    return Ry * v;
}

int main(int argc, char** argv) {
    constexpr int width = 800;
    constexpr int height = 800;

    TGAImage framebuffer(width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

    Model model = Model("./obj/diablo3_pose/diablo3_pose.obj");

    render_view(model, zbuffer, framebuffer, width, height);

    framebuffer.write_tga_file("./framebuffer.tga", true, false);
    zbuffer.write_tga_file("./zbuffer.tga", true, false);
    return 0;
}

float tri_area(int ax, int ay, int bx, int by, int cx, int cy) {
    return (ax * (by - cy) + bx * (cy - ay) + cx * (ay - by)) / 2;
}

void draw_triangle(int ax, int ay, int az, int bx, int by, int bz, int cx,
                   int cy, int cz, TGAImage& zbuffer, TGAImage& framebuffer,
                   TGAColor color) {
    float area = tri_area(ax, ay, bx, by, cx, cy);

    if (area <= 0) return;

    int bbminx = std::min(std::min(ax, bx), cx);
    int bbminy = std::min(std::min(ay, by), cy);
    int bbmaxx = std::max(std::max(ax, bx), cx);
    int bbmaxy = std::max(std::max(ay, by), cy);

#pragma omp parallel for
    for (int px = bbminx; px < bbmaxx; ++px) {
        for (int py = bbminy; py < bbmaxy; ++py) {
            double a = tri_area(px, py, bx, by, cx, cy) / area;
            double b = tri_area(px, py, cx, cy, ax, ay) / area;
            double g = tri_area(px, py, ax, ay, bx, by) / area;

            if (a < 0 || b < 0 || g < 0) continue;

            unsigned char z =
                static_cast<unsigned char>(a * az + b * bz + g * cz);
            if (z <= zbuffer.get(px, py)[0]) continue;
            zbuffer.set(px, py, {z});
            framebuffer.set(px, py, color);
        }
    }
}

void line(int ax, int ay, int bx, int by, TGAImage& framebuffer,
          TGAColor color) {
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

std::tuple<int, int, int> project_vert(vec3 point, int width, int height) {
    int x = static_cast<int>((point.x + 1.0) * width / 2.0);
    int y = static_cast<int>((point.y + 1.0) * height / 2.0);
    int z = static_cast<int>((point.z + 1.0) * 255.0 / 2.0);
    return {x, y, z};
}

void render_view(Model model, TGAImage& zbuffer, TGAImage& framebuffer,
                 int width, int height) {
    std::vector<Triangle3D*> sorted_tris;

    for (Triangle3D const triangle : model.triangles) {
        auto [t1x, t1y, t1z] = project_vert(rot(triangle.p1), width, height);
        auto [t2x, t2y, t2z] = project_vert(rot(triangle.p2), width, height);
        auto [t3x, t3y, t3z] = project_vert(rot(triangle.p3), width, height);

        TGAColor rnd;
        for (int c = 0; c < 3; c++) rnd[c] = std::rand() % 255;

        draw_triangle(t1x, t1y, t1z, t2x, t2y, t2z, t3x, t3y, t3z, zbuffer,
                      framebuffer, rnd);
    }
}