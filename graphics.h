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
// clang-format on

void perspective(const double f) {
    Perspective = {
        {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, -1 / f, 1}}};
}

void viewport(const int x, const int y, const int w, const int h) {
    Viewport = {{{w / 2., 0, 0, x + w / 2.},
                 {0, h / 2., 0, y + h / 2.},
                 {0, 0, 1, 0},
                 {0, 0, 0, 1}}};
}

float tri_area(int ax, int ay, int bx, int by, int cx, int cy) {
    return (ax * (by - cy) + bx * (cy - ay) + cx * (ay - by)) / 2;
}

void draw_triangle(const vec4 clip[3], TGAImage& zbuffer, TGAImage& framebuffer,
                   TGAColor color) {
    vec4 ndc[3] = {clip[0] / clip[0].w, clip[1] / clip[1].w,
                   clip[2] / clip[2].w};
    vec2 screen[3] = {(Viewport * ndc[0]).xy(), (Viewport * ndc[1]).xy(),
                      (Viewport * ndc[2]).xy()};

    mat<3> ABC = {{{screen[0].x, screen[0].y, 1.},
                   {screen[1].x, screen[1].y, 1.},
                   {screen[2].x, screen[2].y, 1.}}};
    if (ABC.det() < 1) return;

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

            z = (z + 1.0) * 127.5;

            if (z <= zbuffer.get(px, py)[0]) continue;
            if (z > 255) z = 255;

            unsigned char cz = static_cast<unsigned char>(z);
            zbuffer.set(px, py, {cz});
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

void rasterize(Model model, Camera& camera, TGAImage& zbuffer,
               TGAImage& framebuffer, int width, int height) {
    std::vector<Triangle3D*> sorted_tris;

    lookat(camera.eye, camera.center, camera.up);
    perspective(norm(camera.eye - camera.center));
    viewport(width / 16, height / 16, width * 7 / 8, height * 7 / 8);

    for (Triangle3D const triangle : model.triangles) {
        vec4 clip[3];
        TGAColor rnd;
        for (int c = 0; c < 3; c++) rnd[c] = std::rand() % 255;

        int i = 0;
        for (vec3 const& vert : {triangle.p1, triangle.p2, triangle.p3})
            clip[i++] =
                Perspective * ModelView * vec4{vert.x, vert.y, vert.z, 1.};

        draw_triangle(clip, zbuffer, framebuffer, rnd);
    }
}
