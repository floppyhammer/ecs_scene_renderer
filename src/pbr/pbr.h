#ifndef ESR_PBR_H
#define ESR_PBR_H

#include "../app/app.h"
#include "../app/plugin.h"
#include "../asset/asset.h"
#include "../ecs/bundle.h"
#include "../ecs/world.h"
#include "../render/render.h"
#include "material.h"

namespace esr {

class PbrPlugin : public Plugin {
    PLUGIN_INIT

    void build(App &app) override;
};

/// A surface consists of a mesh and a material.
class MeshBundle : public Bundle {
public:
    MeshBundle() = default;

    C_AssetHandle<Mesh<Vertex>> mesh;
    C_AssetHandle<StandardMaterial> material;
    C_Transform transform;
    C_GlobalTransform global_transform;
    C_Visibility visibility;

    Entity spawn(World &world) override;
};

} // namespace esr

#endif // ESR_PBR_H
