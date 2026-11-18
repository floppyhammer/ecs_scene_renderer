#ifndef ESR_RENDER_COLOR_H
#define ESR_RENDER_COLOR_H

#include <cstdint>

#include "../geometry/vec4.h"

namespace esr {

struct Color {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 0;

    Color() = default;

    explicit Color(uint32_t _color);

    Color(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a);

    static Color rgb(uint8_t _r, uint8_t _g, uint8_t _b) {
        return {_r, _g, _b, 255};
    }

    static Color rgba(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a) {
        return {_r, _g, _b, _a};
    }

    static Color rgb_linear(float _r, float _g, float _b) {
        return rgba_linear(_r, _g, _b, 255);
    }

    static Color rgba_linear(float _r, float _g, float _b, float _a) {
        auto a = static_cast<uint8_t>(_a * 255.f);
        auto b = static_cast<uint8_t>(_b * 255.f);
        auto g = static_cast<uint8_t>(_g * 255.f);
        auto r = static_cast<uint8_t>(_r * 255.f);
        return {r, g, b, a};
    }

    static Color vec_linear(const Vec4F& v) {
        return rgba_linear(v.x, v.y, v.z, v.w);
    }

    Color lerp(const Color& other, float t) const;

    uint32_t to_u32() const;

    Vec4F to_vec() const;

    /// Check for transparency.
    bool is_opaque() const;

    inline bool operator<(const Color& rhs) const {
        return to_u32() < rhs.to_u32();
    }

    inline Color operator*(Color other) const {
        return rgba_linear(r / 255.0 * other.r / 255.0,
                           g / 255.0 * other.g / 255.0,
                           b / 255.0 * other.b / 255.0,
                           a / 255.0 * other.a / 255.0);
    }

    static Color red() {
        return {255, 0, 0, 255};
    }

    static Color green() {
        return {0, 255, 0, 255};
    }

    static Color blue() {
        return {0, 0, 255, 255};
    }

    static Color white() {
        return {255, 255, 255, 255};
    }

    static Color black() {
        return {0, 0, 0, 255};
    }

    static Color transparent_black() {
        return {0, 0, 0, 0};
    }
};

} // namespace esr

#endif // ESR_RENDER_COLOR_H
