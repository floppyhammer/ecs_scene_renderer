#ifndef ESR_GEOMETRY_VEC4_H
#define ESR_GEOMETRY_VEC4_H

#include <cassert>
#include <optional>

#include "vec2.h"
#include "vec3.h"

namespace esr {

template <typename T>
struct Vec4 {
    T x = 0;
    T y = 0;
    T z = 0;
    T w = 0;

    Vec4() = default;

    explicit Vec4(T s) : x(s), y(s), z(s), w(s){};

    Vec4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w){};

    Vec4(Vec2<T> v, T z, T w) : x(v.x), y(v.y), z(z), w(w){};

    Vec4(Vec3<T> v, T w) : x(v.x), y(v.y), z(v.z), w(w){};

    static Vec4 X() {
        return {1, 0, 0, 0};
    }

    static Vec4 Y() {
        return {0, 1, 0, 0};
    }

    static Vec4 Z() {
        return {0, 0, 1, 0};
    }

    static Vec4 W() {
        return {0, 0, 0, 1};
    }

    Vec2<T> xy() const {
        return {x, y};
    }

    Vec3<T> xyz() const {
        return {x, y, z};
    }

    T square_length() const {
        return x * x + y * y + z * z + w * w;
    }

    float length() const {
        return std::sqrt(square_length());
    }

    /// Returns `self` normalized to length 1.0 if possible, else returns `None`.
    ///
    /// In particular, if the input is zero (or very close to zero), or non-finite,
    /// the result of this operation will be `None`.
    std::optional<Vec4> try_normalize() {
        auto length2 = length();
        if (length2 > 0) {
            return *this / length2;
        }
        return {};
    }

    /// Computes the cross product of `self` and `rhs`.
    Vec4 cross(const Vec4 &rhs) {
        return {
            y * rhs.z - rhs.y * z,
            z * rhs.x - rhs.z * x,
            x * rhs.y - rhs.x * y,
            w * rhs.w - rhs.w * w,
        };
    }
    /// Returns whether `self` is length `1.0` or not.
    ///
    /// Uses a precision threshold of `1e-6`.

    bool is_normalized() {
        return std::abs(square_length() - 1.0) <= EPSILON;
    }

    Vec4 operator+(T s) const {
        return {x + s, y + s, z + s, w + s};
    }

    Vec4 operator-(T s) const {
        return {x - s, y - s, z - s, w - s};
    }

    Vec4 operator/(T s) const {
        return {x / s, y / s, z / s, w / s};
    }

    Vec4 operator*(T s) const {
        return {x * s, y * s, z * s, w * s};
    }

    Vec4 operator+(const Vec4 &rhs) const {
        return {x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w};
    }

    Vec4 operator-(const Vec4 &rhs) const {
        return {x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w};
    }

    bool operator==(const Vec4 &b) const {
        return x == b.x && y == b.y && z == b.z && w == b.w;
    }

    void operator+=(const Vec4 &b) {
        x += b.x;
        y += b.y;
        z += b.z;
        w += b.w;
    }
};

template <typename T>
Vec4<T> operator-(const Vec4<T> &obj) {
    return {-obj.x, -obj.y, -obj.z, -obj.w};
}

typedef Vec4<float> Vec4F;

} // namespace esr

#endif // ESR_GEOMETRY_Vec4_H
