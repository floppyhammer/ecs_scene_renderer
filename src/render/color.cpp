#include "color.h"

#include <algorithm>

namespace esr {

Color::Color(uint32_t _color) {
    // Note that the order is reversed.
    a = _color >> 24u;
    b = (_color << 8u) >> 24u;
    g = (_color << 16u) >> 24u;
    r = (_color << 24u) >> 24u;
}

Color::Color(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a) : r(_r), g(_g), b(_b), a(_a) {
}

uint32_t Color::to_u32() const {
    uint32_t rgba = (r << 24u) + (g << 16u) + (b << 8u) + a;

    return rgba;
}

bool Color::is_opaque() const {
    return a != 0;
}

Color Color::lerp(const Color& other, float t) const {
    t = std::max(std::min(t, 0.0f), 1.0f);

    auto vec = to_vec();

    return vec_linear(vec + (other.to_vec() - vec) * t);
}

Vec4F Color::to_vec() const {
    return {r / 255.f, g / 255.f, b / 255.f, a / 255.f};
}

} // namespace esr
