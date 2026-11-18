#ifndef ESR_GEOMETRY_MAT4_H
#define ESR_GEOMETRY_MAT4_H

#include <array>

#include "vec3.h"

#undef near
#undef far

namespace esr {

/// A 4x4 matrix, in column-major order.
struct Mat4 {
    std::array<float, 16> v{};

    Mat4() = default;

    Mat4(std::array<float, 16> values) {
        v = values;
    }

    explicit Mat4(float s) {
        for (int i = 0; i < 4; i++) {
            v[i * 4 + i] = s;
        }
    }

    static Mat4 from_scale(const Vec3F &scale) {
        auto mat = Mat4(1);

        mat.v[0] = scale.x;
        mat.v[5] = scale.y;
        mat.v[10] = scale.z;

        return mat;
    }

    static Mat4 from_translation(const Vec3F &translation) {
        auto mat = Mat4(1);

        mat.v[12] = translation.x;
        mat.v[13] = translation.y;
        mat.v[14] = translation.z;

        return mat;
    }

    /// Creates an infinite reverse right-handed perspective projection matrix with `[0,1]` depth range.
    static Mat4 perspective_infinite_reverse_rh(float fov_y_radians, float aspect_ratio, float z_near, float z_far) {
        assert(z_near > 0.0);

        float f = 1.0 / std::tan(0.5 * fov_y_radians);

        // See
        // https://gamedev.stackexchange.com/questions/120338/what-does-a-perspective-projection-matrix-look-like-in-opengl
        std::array<float, 16> v = {f / aspect_ratio,
                                   0.0,
                                   0.0,
                                   0.0,
                                   0.0,
                                   f,
                                   0.0,
                                   0.0,
                                   0.0,
                                   0.0,
                                   -(z_far + z_near) / (z_far - z_near),
                                   -1.0,
                                   0.0,
                                   0.0,
                                   -(2.0f * z_far * z_near) / (z_far - z_near),
                                   0.0};
        return {v};
    }

    /// Creates a right-handed orthographic projection matrix with `[0,1]` depth range.
    static Mat4 orthographic_rh(float left, float right, float bottom, float top, float near, float far) {
        float rcp_width = 1.0 / (right - left);
        float rcp_height = 1.0 / (top - bottom);
        float r = 1.0 / (near - far);
        std::array<float, 16> v = {rcp_width + rcp_width,
                                   0.0,
                                   0.0,
                                   0.0,
                                   0.0,
                                   rcp_height + rcp_height,
                                   0.0,
                                   0.0,
                                   0.0,
                                   0.0,
                                   r,
                                   0.0,
                                   -(left + right) * rcp_width,
                                   -(top + bottom) * rcp_height,
                                   r * near,
                                   1.0};
        return {v};
    }

    Mat4 operator*(const Mat4 &other) const {
        auto mat = Mat4();

        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                auto index = i + j * 4;
                mat.v[index] = 0;
                for (int k = 0; k < 4; k++) {
                    mat.v[index] += v[i + k * 4] * other.v[k + j * 4];
                }
            }
        }

        return mat;
    }

    Mat4 translate(const Vec3F &translation) const {
        return *this * from_translation(translation);
    }

    Mat4 scale(const Vec3F &scale) const {
        return *this * from_scale(scale);
    }

    float get_element(uint32_t row, uint32_t col) const {
        return v[col * 4 + row];
    }

    void set_element(uint32_t row, uint32_t col, float value) {
        v[col * 4 + row] = value;
    }

    Mat4 transpose() const {
        Mat4 new_mat;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                new_mat.set_element(j, i, get_element(i, j));
            }
        }
        return new_mat;
    }

    Mat4 inverse() const {
        std::array<float, 16> inv;
        float det;

        inv[0] = v[5] * v[10] * v[15] - v[5] * v[11] * v[14] - v[9] * v[6] * v[15] + v[9] * v[7] * v[14] +
                 v[13] * v[6] * v[11] - v[13] * v[7] * v[10];

        inv[4] = -v[4] * v[10] * v[15] + v[4] * v[11] * v[14] + v[8] * v[6] * v[15] - v[8] * v[7] * v[14] -
                 v[12] * v[6] * v[11] + v[12] * v[7] * v[10];

        inv[8] = v[4] * v[9] * v[15] - v[4] * v[11] * v[13] - v[8] * v[5] * v[15] + v[8] * v[7] * v[13] +
                 v[12] * v[5] * v[11] - v[12] * v[7] * v[9];

        inv[12] = -v[4] * v[9] * v[14] + v[4] * v[10] * v[13] + v[8] * v[5] * v[14] - v[8] * v[6] * v[13] -
                  v[12] * v[5] * v[10] + v[12] * v[6] * v[9];

        inv[1] = -v[1] * v[10] * v[15] + v[1] * v[11] * v[14] + v[9] * v[2] * v[15] - v[9] * v[3] * v[14] -
                 v[13] * v[2] * v[11] + v[13] * v[3] * v[10];

        inv[5] = v[0] * v[10] * v[15] - v[0] * v[11] * v[14] - v[8] * v[2] * v[15] + v[8] * v[3] * v[14] +
                 v[12] * v[2] * v[11] - v[12] * v[3] * v[10];

        inv[9] = -v[0] * v[9] * v[15] + v[0] * v[11] * v[13] + v[8] * v[1] * v[15] - v[8] * v[3] * v[13] -
                 v[12] * v[1] * v[11] + v[12] * v[3] * v[9];

        inv[13] = v[0] * v[9] * v[14] - v[0] * v[10] * v[13] - v[8] * v[1] * v[14] + v[8] * v[2] * v[13] +
                  v[12] * v[1] * v[10] - v[12] * v[2] * v[9];

        inv[2] = v[1] * v[6] * v[15] - v[1] * v[7] * v[14] - v[5] * v[2] * v[15] + v[5] * v[3] * v[14] +
                 v[13] * v[2] * v[7] - v[13] * v[3] * v[6];

        inv[6] = -v[0] * v[6] * v[15] + v[0] * v[7] * v[14] + v[4] * v[2] * v[15] - v[4] * v[3] * v[14] -
                 v[12] * v[2] * v[7] + v[12] * v[3] * v[6];

        inv[10] = v[0] * v[5] * v[15] - v[0] * v[7] * v[13] - v[4] * v[1] * v[15] + v[4] * v[3] * v[13] +
                  v[12] * v[1] * v[7] - v[12] * v[3] * v[5];

        inv[14] = -v[0] * v[5] * v[14] + v[0] * v[6] * v[13] + v[4] * v[1] * v[14] - v[4] * v[2] * v[13] -
                  v[12] * v[1] * v[6] + v[12] * v[2] * v[5];

        inv[3] = -v[1] * v[6] * v[11] + v[1] * v[7] * v[10] + v[5] * v[2] * v[11] - v[5] * v[3] * v[10] -
                 v[9] * v[2] * v[7] + v[9] * v[3] * v[6];

        inv[7] = v[0] * v[6] * v[11] - v[0] * v[7] * v[10] - v[4] * v[2] * v[11] + v[4] * v[3] * v[10] +
                 v[8] * v[2] * v[7] - v[8] * v[3] * v[6];

        inv[11] = -v[0] * v[5] * v[11] + v[0] * v[7] * v[9] + v[4] * v[1] * v[11] - v[4] * v[3] * v[9] -
                  v[8] * v[1] * v[7] + v[8] * v[3] * v[5];

        inv[15] = v[0] * v[5] * v[10] - v[0] * v[6] * v[9] - v[4] * v[1] * v[10] + v[4] * v[2] * v[9] +
                  v[8] * v[1] * v[6] - v[8] * v[2] * v[5];

        det = v[0] * inv[0] + v[1] * inv[4] + v[2] * inv[8] + v[3] * inv[12];

        assert(det != 0);

        det = 1.0 / det;

        for (int i = 0; i < 16; i++) {
            inv[i] *= det;
        }

        return {inv};
    }
};

} // namespace esr

#endif // ESR_GEOMETRY_MAT4_H
