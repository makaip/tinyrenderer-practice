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

int main(int argc, char** argv) {
    constexpr int width = 800;
    constexpr int height = 800;
    
    TGAImage framebuffer(width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

    Model model = Model("./obj/diablo3_pose/diablo3_pose.obj");
    Camera camera;

    rasterize(model, camera, zbuffer, framebuffer, width, height);

    framebuffer.write_tga_file("./framebuffer.tga", true, false);
    zbuffer.write_tga_file("./zbuffer.tga", true, false);
    return 0;
}
