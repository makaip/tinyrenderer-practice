#define _USE_MATH_DEFINES

#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "camera.h"
#include "graphics.h"
#include "model.h"
#include "tgaimage.h"

// clang-format off
constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};
// clang-format on

struct RandomShader : IShader {
    const Model& model;
    vec3 tri[3];

    RandomShader(const Model& m) : model(m) {}

    virtual vec4 vertex(const int vert) {
        vec3 v = model.vertices[vert];
        vec4 gl_Position = ModelView * vec4{v.x, v.y, v.z, 1.};
        tri[vert] = gl_Position.xyz();
        return Perspective * gl_Position;
    }

    virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const {
        return {false, color};
    }
};

int main(int argc, char** argv) {
    constexpr int width = 800;
    constexpr int height = 800;

    TGAImage framebuffer(width, height, TGAImage::RGB);
    Model model = Model("F:/Programming/GitHub/tinyrenderer-practice/obj/diablo3_pose/diablo3_pose.obj");
    RandomShader shader(model);
    Camera camera;

    rasterize(model, camera, shader, framebuffer, width, height);

    framebuffer.write_tga_file("./framebuffer.tga", true, false);
    return 0;
}
