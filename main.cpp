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

    const TGAImage& albedo_map;
    const TGAImage& norm_map;
    const TGAImage& spec_map;

    vec3 light;
    vec3 tri[3];
    vec3 norm[3];
    vec3 alb[3];
    vec2 uv[3];

    PhongShader(const vec3 light_dir, const Model& m, const TGAImage& a,
                const TGAImage& n, const TGAImage& s)
        : model(m), albedo_map(a), norm_map(n), spec_map(s) {
        light = normalize(
            (ModelView * vec4{light_dir.x, light_dir.y, light_dir.z, 0.})
                .xyz());
    }

    virtual vec4 vertex(const int nthvert, const Point3D& v) override {
        vec4 gl_pos = ModelView * vec4{v.pos.x, v.pos.y, v.pos.z, 1.};
        vec4 gl_norm = ModelView * vec4{v.norm.x, v.norm.y, v.norm.z, 0.};

        tri[nthvert] = gl_pos.xyz();
        norm[nthvert] = gl_norm.xyz();
        uv[nthvert] = v.uv;

        return Perspective * gl_pos;
    }

    virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const {
        TGAColor col = {255, 255, 255, 255};
        double ambient = 0.2;

        vec3 e0 = tri[1] - tri[0];
        vec3 e1 = tri[2] - tri[0];
        vec2 u0 = uv[1] - uv[0];
        vec2 u1 = uv[2] - uv[0];

        // clang-format off
        mat<3,2> E = {{e0.x, e1.x},
                      {e0.y, e1.y},
                      {e0.z, e1.z}};

        mat<2> U = {{u0.x, u0.y},
                    {u1.x, u1.y}};
        // clang-format on

        mat<3, 2> tb = E * inverse(U).transpose();

        vec3 face_tang =   normalize(vec3{tb[0][0], tb[1][0], tb[2][0]});
        vec3 face_bitang = normalize(vec3{tb[0][1], tb[1][1], tb[2][1]});
        vec3 interp_normal =
            normalize(bar.x * norm[0] + bar.y * norm[1] + bar.z * norm[2]);

        // clang-format off
        mat<4, 3> D = {
            {face_tang.x,     face_tang.y,     face_tang.z},
            {face_bitang.x,   face_bitang.y,   face_bitang.z},
            {interp_normal.x, interp_normal.y, interp_normal.z},
            {0,               0,               1}
        };
        // clang-format on

        vec2 uv_coords = bar.x * uv[0] + bar.y * uv[1] + bar.z * uv[2];

        TGAColor a_color =
            sample2D(albedo_map, vec2{uv_coords.x, 1.0 - uv_coords.y});
        TGAColor n_color =
            sample2D(norm_map, vec2{uv_coords.x, 1.0 - uv_coords.y});
        TGAColor s_color =
            sample2D(spec_map, vec2{uv_coords.x, 1.0 - uv_coords.y});

        vec3 albedo = {a_color[0] / 255.0, a_color[1] / 255.0,
                       a_color[2] / 255.0};
        vec3 map_normal = {n_color[2] / 255.0 * 2.0 - 1.0,
                           n_color[1] / 255.0 * 2.0 - 1.0,
                           n_color[0] / 255.0 * 2.0 - 1.0};
        double map_spec = s_color[0] / 255.0;

        vec3 normal = normalize(D.transpose() * vec4{map_normal.x, map_normal.y, map_normal.z, 0.});

        double normlight = dot(normal, light);
        vec3 reflection = normalize(normal * normlight * 2 - light);
        double diffuse = std::max(0.0, normlight);
        double specular = std::pow(std::max(reflection.z, 0.0), 35);

        for (int channel : {0, 1, 2}) {
            double shaded_color = (ambient + 0.6 * diffuse) * albedo[channel] +
                                  (specular * map_spec);
            col[channel] = 255.0 * std::min(1.0, shaded_color);
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

    TGAImage normal_map;
    normal_map.read_tga_file(
        "F:/Programming/GitHub/tinyrenderer-practice/obj/african_head/"
        "african_head_nm_tangent.tga");

    TGAImage albedo_map;
    albedo_map.read_tga_file(
        "F:/Programming/GitHub/tinyrenderer-practice/obj/african_head/"
        "african_head_diffuse.tga");

    TGAImage specular_map;
    specular_map.read_tga_file(
        "F:/Programming/GitHub/tinyrenderer-practice/obj/african_head/"
        "african_head_spec.tga");

    vec3 light = {1, 0.5, 0.5};
    Camera camera;

    lookat(camera.eye, camera.center, camera.up);
    init_perspective(norm(camera.eye - camera.center));
    init_viewport(width / 16, height / 16, width * 7 / 8, height * 7 / 8);
    init_zbuffer(width, height);

    PhongShader shader(light, model, albedo_map, normal_map, specular_map);
    rasterize(model, camera, shader, framebuffer, width, height);

    framebuffer.write_tga_file("./framebuffer.tga", true, false);
    return 0;
}
