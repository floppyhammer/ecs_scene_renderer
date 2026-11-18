#ifndef ESR_GEOMETRY_MAT3_H
#define ESR_GEOMETRY_MAT3_H

#include <array>

#include "vec3.h"

namespace esr {

/// A 3x3 matrix, in column-major order.
struct Mat3 {
    std::array<float, 9> v{};

    Mat3() = default;

    Mat3(const std::array<float, 9> &v) : v(v){};

    explicit Mat3(float s) {
        for (int i = 0; i < 3; i++) {
            v[i * 3 + i] = s;
        }
    }

    static Mat3 from_scale(const Vec3F &scale) {
        auto mat = Mat3(1);

        mat.v[0] = scale.x;
        mat.v[4] = scale.y;
        mat.v[8] = scale.z;

        return mat;
    }

    static Mat3 from_cols(const Vec3F &c0, const Vec3F &c1, const Vec3F &c2) {
        auto mat = Mat3(0);

        mat.v[0] = c0.x;
        mat.v[1] = c0.y;
        mat.v[2] = c0.z;

        mat.v[3] = c1.x;
        mat.v[4] = c1.y;
        mat.v[5] = c1.z;

        mat.v[6] = c2.x;
        mat.v[7] = c2.y;
        mat.v[8] = c2.z;

        return mat;
    }

    Mat3 scale(const Vec3F &scale) const {
        return *this * from_scale(scale);
    }

    Mat3 operator*(const Mat3 &rhs) const {
        auto mat = Mat3();

        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                auto index = i + j * 3;
                mat.v[index] = 0;
                for (int k = 0; k < 3; k++) {
                    mat.v[index] += v[i + k * 3] * rhs.v[k + j * 3];
                }
            }
        }

        return mat;
    }

    Vec3F operator*(const Vec3F &rhs) const {
        auto new_vec = Vec3F();

        new_vec.x = v[0] * rhs.x + v[3] * rhs.y + v[6] * rhs.z;
        new_vec.y = v[1] * rhs.x + v[4] * rhs.y + v[7] * rhs.z;
        new_vec.z = v[2] * rhs.x + v[5] * rhs.y + v[8] * rhs.z;

        return new_vec;
    }
};

} // namespace esr

#endif // ESR_GEOMETRY_MAT3_H
