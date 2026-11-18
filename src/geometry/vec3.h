#ifndef ESR_GEOMETRY_VEC3_H
#define ESR_GEOMETRY_VEC3_H

#include <cassert>

#include "basic.h"
#include "vec2.h"

namespace esr {

template <typename T>
struct Vec3 {
    T x = 0;
    T y = 0;
    T z = 0;

    Vec3() = default;

    explicit Vec3(T s) : x(s), y(s), z(s){};

    Vec3(T x, T y, T z) : x(x), y(y), z(z){};

    Vec3(Vec2<T> v, T z) : x(v.x), y(v.y), z(z){};

    static Vec3 X() {
        return {1, 0, 0};
    }

    static Vec3 Y() {
        return {0, 1, 0};
    }

    static Vec3 Z() {
        return {0, 0, 1};
    }

    Vec2<T> xy() const {
        return {x, y};
    }

    T square_length() const {
        return x * x + y * y + z * z;
    }

    float length() const {
        return std::sqrt(square_length());
    }

    /// Returns `self` normalized to length 1.0 if possible, else returns `None`.
    ///
    /// In particular, if the input is zero (or very close to zero), or non-finite,
    /// the result of this operation will be `None`.
    std::optional<Vec3> try_normalize() {
        auto length2 = length();
        if (length2 > 0) {
            return *this / length2;
        }
        return {};
    }

    /// Computes the cross product of `self` and `rhs`.
    Vec3 cross(const Vec3 &rhs) {
        return {
            y * rhs.z - rhs.y * z,
            z * rhs.x - rhs.z * x,
            x * rhs.y - rhs.x * y,
        };
    }
    /// Returns whether `self` is length `1.0` or not.
    ///
    /// Uses a precision threshold of `1e-6`.

    bool is_normalized() {
        return std::abs(square_length() - 1.0) <= EPSILON;
    }

    /// Returns any unit-length vector that is orthogonal to the given one.
    /// The input vector must be finite and non-zero.
    Vec3 any_orthonormal_vector() {
        assert(is_normalized());

        float sign = z >= 0 ? 1 : -1;
        float a = -1.0 / (sign + z);
        float b = x * y * a;

        return {b, sign + y * y * a, -y};
    }

    Vec3 operator+(T s) const {
        return {x + s, y + s, z + s};
    }

    Vec3 operator-(T s) const {
        return {x - s, y - s, z - s};
    }

    Vec3 operator/(T s) const {
        return {x / s, y / s, z / s};
    }

    Vec3 operator*(T s) const {
        return {x * s, y * s, z * s};
    }

    Vec3 operator+(const Vec3 &rhs) const {
        return {x + rhs.x, y + rhs.y, z + rhs.z};
    }

    Vec3 operator-(const Vec3 &rhs) const {
        return {x - rhs.x, y - rhs.y, z - rhs.z};
    }

    bool operator==(const Vec3 &b) const {
        return x == b.x && y == b.y && z == b.z;
    }

    void operator+=(const Vec3 &b) {
        x += b.x;
        y += b.y;
        z += b.z;
    }
};

template <typename T>
Vec3<T> operator-(const Vec3<T> &obj) {
    return {-obj.x, -obj.y, -obj.z};
}

typedef Vec3<float> Vec3F;

} // namespace esr

#endif // ESR_GEOMETRY_VEC3_H
