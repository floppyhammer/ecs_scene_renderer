#ifndef ESR_TEXT_3D_H
#define ESR_TEXT_3D_H

#include "../app/plugin.h"
#include "../asset/asset.h"
#include "../ecs/bundle.h"
#include "../ecs/world.h"
#include "../pbr/material.h"
#include "../render/render.h"
#include "../render/texture/image.h"
#include "../transform/components.h"
#include "font.h"

namespace esr {

class Text3dPlugin : public Plugin {
    PLUGIN_INIT

    void build(App& app) override;
};

struct C_Text3d {
    std::string content;
    Color color;
    float depth = 0.1;
    std::map<uint32_t, C_AssetHandle<Mesh<Vertex>>> glyph_cache;
};

struct C_Text3dGlyph {
    uint32_t utf_char;
};

class Text3dBundle : public Bundle {
public:
    C_Text3d text;
    C_AssetHandle<StandardMaterial> material;
    C_Transform transform;
    C_GlobalTransform global_transform;
    C_AssetHandle<Text3dFont> font;
    C_Visibility visibility;

    Entity spawn(World& world) override;
};

} // namespace esr

#endif // ESR_TEXT_3D_H
