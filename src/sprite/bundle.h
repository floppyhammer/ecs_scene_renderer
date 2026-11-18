#ifndef ESR_SPRITE_BUNDLE_H
#define ESR_SPRITE_BUNDLE_H

#include "../ecs/bundle.h"
#include "../render/render.h"
#include "../render/texture/image.h"
#include "../sprite/sprite.h"
#include "../transform/components.h"

namespace esr {

class World;

class SpriteBundle : public Bundle {
public:
    C_Sprite sprite;
    C_Transform transform;
    C_GlobalTransform global_transform;
    C_AssetHandle<Image> texture;
    C_Visibility visibility;

    Entity spawn(World& world) override;
};

} // namespace esr

#endif // ESR_SPRITE_BUNDLE_H
