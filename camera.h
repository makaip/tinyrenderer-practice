#pragma once
#include "vecs.h"

struct Camera {
    vec3 eye    = {-1, 0, 2};
    vec3 center = {0, 0, 0};
    vec3 up     = {0, 1, 0};
};