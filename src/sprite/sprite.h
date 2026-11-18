#ifndef ESR_SPRITE_H
#define ESR_SPRITE_H

#include <optional>

#include "../app/plugin.h"
#include "../geometry/rect.h"
#include "../render/color.h"

namespace esr {

class SpritePlugin : public Plugin {
    PLUGIN_INIT

    void build(App& app) override;
};

struct C_Sprite {
    /// The sprite's color tint.
    Color color = Color::white();
    /// Flip the sprite along the `X` axis.
    bool flip_x = false;
    /// Flip the sprite along the `Y` axis.
    bool flip_y = false;
    /// An optional custom size for the sprite that will be used when rendering, instead of the size
    /// of the sprite's image.
    std::optional<Vec2F> custom_size;
    /// An optional rectangle representing the region of the sprite's image to render, instead of
    /// rendering the full image. This is an easy one-off alternative to using a texture atlas.
    std::optional<RectF> rect;
};

} // namespace esr

#endif // ESR_SPRITE_H
