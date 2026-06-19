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

struct PhongShader : IShader {
    const Model& model;
    vec3 light;
    vec3 tri[3];
    vec3 norm[3];

    PhongShader(const vec3 light_dir, const Model& m) : model(m) {
        light = normalize(
            (ModelView * vec4{light_dir.x, light_dir.y, light_dir.z, 0.})
                .xyz());
    }
    
    virtual vec4 vertex(const int nthvert, const Point3D& v) override {
        // mat<4> TransposedModelView = ModelView.transpose();

        vec4 gl_pos = ModelView * vec4{v.pos.x, v.pos.y, v.pos.z, 1.};
        vec4 gl_norm = ModelView * vec4{v.norm.x, v.norm.y, v.norm.z, 0.};
        
        tri[nthvert] = gl_pos.xyz();
        norm[nthvert] = gl_norm.xyz();
        
        return Perspective * gl_pos;
    }

    virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const {
        TGAColor col = {255, 255, 255, 255};
        vec3 light = {1, 0, 0};
        double ambient = 0.3;

        // vec3 normal = normalize(cross(tri[1] - tri[0], tri[2] - tri[0]));
        vec3 normal = normalize(bar.x * norm[0] + bar.y * norm[1] + bar.z * norm[2]);
        
        double normlight = dot(normal, light);

        vec3 reflection = normalize(normal * normlight * 2 - light);
        double diffuse = std::max(0.0, normlight);
        double specular = std::pow(std::max(reflection.z, 0.0), 35);

        for (int channel : {0, 1, 2}) {
            col[channel] *=
                std::min(1.0, ambient + 0.6 * diffuse + 0.9 * specular);
        }

        return {false, col};
    }
};

int main(int argc, char** argv) {
    constexpr int width = 800;
    constexpr int height = 800;

    TGAImage framebuffer(width, height, TGAImage::RGB);
    Model model = Model(
        "F:/Programming/GitHub/tinyrenderer-practice/obj/african_head/"
        "african_head.obj");
    vec3 light{1, 1, 1};
    PhongShader shader(light, model);
    Camera camera;

    rasterize(model, camera, shader, framebuffer, width, height);

    framebuffer.write_tga_file("./framebuffer.tga", true, false);
    return 0;
}
