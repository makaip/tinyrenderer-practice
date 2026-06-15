#pragma once

#define _USE_MATH_DEFINES

#include <cassert>
#include <cmath>
#include <iostream>

template <int n>
struct vec {
    double data[n] = {};

    double& operator[](const int i) {
        assert(i >= 0 && i < n);
        return data[i];
    }

    double operator[](const int i) const {
        assert(i >= 0 && i < n);
        return data[i];
    }
};

template <>
struct vec<2> {
    double x = 0, y = 0;

    vec() = default;
    vec(double x, double y) : x(x), y(y) {}

    double& operator[](int i) {
        assert(i >= 0 && i < 2);

        if (i == 0) return x;
        if (i == 1) return y;
    }

    double operator[](int i) const {
        assert(i >= 0 && i < 2);

        if (i == 0) return x;
        if (i == 1) return y;
    }
};

template <>
struct vec<3> {
    double x = 0, y = 0, z = 0;

    double& operator[](const int i) {
        assert(i >= 0 && i < 3);

        if (i == 0) return x;
        if (i == 1) return y;
        if (i == 2) return z;
    }

    double operator[](const int i) const {
        assert(i >= 0 && i < 3);

        if (i == 0) return x;
        if (i == 1) return y;
        if (i == 2) return z;
    }
};

template <>
struct vec<4> {
    double x = 0, y = 0, z = 0, w = 0;

    vec() = default;
    vec(double x, double y, double z, double w) : x(x), y(y), z(z), w(w) {}

    double& operator[](int i) {
        assert(i >= 0 && i < 4);

        if (i == 0) return x;
        if (i == 1) return y;
        if (i == 2) return z;
        if (i == 3) return w;
    }
    double operator[](int i) const {
        assert(i >= 0 && i < 4);

        if (i == 0) return x;
        if (i == 1) return y;
        if (i == 2) return z;
        if (i == 3) return w;
    }
};

typedef vec<2> vec2;
typedef vec<3> vec3;
typedef vec<4> vec4;

template <int n>
vec<n> operator+(const vec<n>& u, const vec<n>& v) {
    vec<n> result;
    for (int i = 0; i < n; i++) result[i] = u[i] + v[i];
    return result;
}

template <int n>
vec<n> operator-(const vec<n>& u, const vec<n>& v) {
    vec<n> result;
    for (int i = 0; i < n; i++) result[i] = u[i] - v[i];
    return result;
}

template <int n>
vec<n> operator-(const vec<n>& v) {
    vec<n> result;
    for (int i = 0; i < n; i++) result[i] = -v[i];
    return result;
}

template <int n>
vec<n> operator*(const vec<n>& v, const double t) {
    vec<n> result;
    for (int i = 0; i < n; i++) result[i] = v[i] * t;
    return result;
}

template <int n>
vec<n> operator*(const double t, const vec<n>& v) {
    return v * t;
}

template <int n>
vec<n> operator/(const vec<n>& v, const double t) {
    vec<n> result;
    for (int i = 0; i < n; i++) result[i] = v[i] / t;
    return result;
}

template <int n>
vec<n> operator/(const double t, const vec<n>& v) {
    vec<n> result;
    for (int i = 0; i < n; i++) result[i] = t / v[i];
    return result;
}

template <int n>
double dot(const vec<n>& u, const vec<n>& v) {
    double sum = 0;
    for (int i = 0; i < n; i++) sum += u[i] * v[i];
    return sum;
}

vec3 cross(const vec3& u, const vec3& v) {
    return vec3(u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z,
                u.x * v.y - u.y * v.x);
}

template <int n>
double norm2(const vec<n>& v) {
    return dot(v, v);
}

template <int n>
double norm(const vec<n>& v) {
    return std::sqrt(norm2(v));
}

template <int n>
vec<n> normalize(const vec<n>& v) {
    return v / norm(v);
}