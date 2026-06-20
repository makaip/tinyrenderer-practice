#pragma once

#define _USE_MATH_DEFINES

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "camera.h"
#include "mats.h"
#include "model.h"
#include "tgaimage.h"
#include "vecs.h"

mat<4> ModelView, Viewport, Perspective;
std::vector<double> zbuffer;

struct IShader {
    TGAColor color = {};
    virtual vec4 vertex(const int nthvert, const Point3D& v) = 0;
    virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const = 0;

    static TGAColor sample2D(const TGAImage& img, const vec2& uvf) {
        return img.get(uvf[0] * img.width(), uvf[1] * img.height());
    }
};

// clang-format off
vec3 rot(vec3 v) {
    const double a = M_PI / 6;
    mat<3> Ry = {{
        {std::cos(a),   0,  std::sin(a)},
        {0,             1,  0},
        {-std::sin(a),  0,  std::cos(a)}
    }};
    
    return Ry * v;
}

void init_perspective(const double f) {
    Perspective = {{
        {1,  0,  0,      0}, 
        {0,  1,  0,      0}, 
        {0,  0,  1,      0}, 
        {0,  0,  -1 / f, 1}
    }};
}
// clang-format on

void init_viewport(const int x, const int y, const int w, const int h) {
    Viewport = {{{w / 2., 0, 0, x + w / 2.},
                 {0, h / 2., 0, y + h / 2.},
                 {0, 0, 1, 0},
                 {0, 0, 0, 1}}};
}

void init_zbuffer(const int width, const int height) {
    zbuffer = std::vector(width * height, -1000.);
}

void draw_triangle(const vec4 clip[3], const IShader& shader,
                   TGAImage& framebuffer) {
    vec4 ndc[3] = {clip[0] / clip[0].w, clip[1] / clip[1].w,
                   clip[2] / clip[2].w};
    vec2 screen[3] = {(Viewport * ndc[0]).xy(), (Viewport * ndc[1]).xy(),
                      (Viewport * ndc[2]).xy()};

    mat<3> ABC = {{{screen[0].x, screen[0].y, 1.},
                   {screen[1].x, screen[1].y, 1.},
                   {screen[2].x, screen[2].y, 1.}}};
    if (ABC.det() <= 0) return;

    auto [bbminx, bbmaxx] =
        std::minmax({screen[0].x, screen[1].x, screen[2].x});
    auto [bbminy, bbmaxy] =
        std::minmax({screen[0].y, screen[1].y, screen[2].y});

    mat<3> ABC_it = inverse(ABC).transpose();

#pragma omp parallel for
    for (int px = std::max<int>(bbminx, 0);
         px <= std::min<int>(bbmaxx, framebuffer.width() - 1); px++) {
        for (int py = std::max<int>(bbminy, 0);
             py <= std::min<int>(bbmaxy, framebuffer.height() - 1); py++) {
            vec3 bc = ABC_it * vec3{static_cast<double>(px),
                                    static_cast<double>(py), 1.};

            if (bc.x < 0 || bc.y < 0 || bc.z < 0) continue;

            double z = dot(bc, vec3{ndc[0].z, ndc[1].z, ndc[2].z});
            if (z <= zbuffer.at(px + py * framebuffer.width())) continue;

            vec3 bc_clip = {bc.x / clip[0].w, bc.y / clip[1].w,
                            bc.z / clip[2].w};
            bc_clip = bc_clip / (bc_clip.x + bc_clip.y + bc_clip.z);

            auto frag = shader.fragment(bc_clip);
            bool discard = std::get<0>(frag);
            TGAColor color = std::get<1>(frag);

            if (discard) continue;

            zbuffer.at(px + py * framebuffer.width()) = z;
            framebuffer.set(px, py, color);
        }
    }
}

void lookat(const vec3 eye, const vec3 center, const vec3 up) {
    vec3 n = normalize(eye - center);
    vec3 l = normalize(cross(up, n));
    vec3 m = normalize(cross(n, l));

    ModelView = mat<4>{{{l.x, l.y, l.z, 0},
                        {m.x, m.y, m.z, 0},
                        {n.x, n.y, n.z, 0},
                        {0, 0, 0, 1}}} *
                mat<4>{{{1, 0, 0, -center.x},
                        {0, 1, 0, -center.y},
                        {0, 0, 1, -center.z},
                        {0, 0, 0, 1}}};
}

void rasterize(Model& model, Camera& camera, IShader& shader,
               TGAImage& framebuffer, int width, int height) {
    for (Triangle3D const& triangle : model.triangles) {
        vec4 clip[3];

        size_t indices[3] = {triangle.p1, triangle.p2, triangle.p3};
        for (int i = 0; i < 3; i++) {
            clip[i] = shader.vertex(i, model.vertices[indices[i]]);
        }

        draw_triangle(clip, shader, framebuffer);
    }
}
