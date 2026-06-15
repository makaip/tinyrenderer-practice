#pragma once

#include <cassert>
#include <cmath>
#include <iostream>

#include "vecs.h"

template <int n>
struct mat {
    vec<n> rows[n];
    mat() = default;

    mat(std::initializer_list<std::initializer_list<double>> init) {
        assert(init.size() == n);
        int i = 0;
        for (auto& row : init) {
            assert(row.size() == n);
            int j = 0;
            for (auto& val : row) {
                rows[i][j++] = val;
            }
            i++;
        }
    }

    vec<n>& operator[](int i) {
        assert(i >= 0 && i < n);
        return rows[i];
    }

    const vec<n>& operator[](int i) const {
        assert(i >= 0 && i < n);
        return rows[i];
    }

    static mat identity() {
        mat m;
        for (int i = 0; i < n; i++) m[i][i] = 1.0;
        return m;
    }

    vec<n> col(int j) const {
        assert(j >= 0 && j < n);
        vec<n> c;
        for (int i = 0; i < n; i++) c[i] = rows[i][j];
        return c;
    }

    mat transpose() const {
        mat result;
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++) result[i][j] = rows[j][i];
        return result;
    }
};

template <int n>
mat<n> operator*(const mat<n>& A, const mat<n>& B) {
    mat<n> result;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) result[i][j] = dot(A[i], B.col(j));
    return result;
}

template <int n>
vec<n> operator*(const mat<n>& A, const vec<n>& v) {
    vec<n> result;
    for (int i = 0; i < n; i++) result[i] = dot(A[i], v);
    return result;
}

template <int n>
std::ostream& operator<<(std::ostream& out, const mat<n>& M) {
    for (int i = 0; i < n; i++) out << M[i] << "\n";
    return out;
}

template <int n>
mat<n> inverse(const mat<n>& A) {
    double aug[n][2 * n] = {};
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) aug[i][j] = A[i][j];
        aug[i][n + i] = 1.0;
    }

    for (int col = 0; col < n; col++) {
        int pivot = col;
        for (int row = col + 1; row < n; row++)
            if (std::abs(aug[row][col]) > std::abs(aug[pivot][col]))
                pivot = row;

        for (int j = 0; j < 2 * n; j++) std::swap(aug[col][j], aug[pivot][j]);

        assert(std::abs(aug[col][col]) > 1e-12 &&
               "Not invertible. Matrix is singular");

        double scale = aug[col][col];
        for (int j = 0; j < 2 * n; j++) aug[col][j] /= scale;

        for (int row = 0; row < n; row++) {
            if (row == col) continue;
            double factor = aug[row][col];
            for (int j = 0; j < 2 * n; j++) aug[row][j] -= factor * aug[col][j];
        }
    }

    mat<n> result;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) result[i][j] = aug[i][n + j];
    return result;
}