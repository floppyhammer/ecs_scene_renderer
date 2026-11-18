#ifndef ESR_RENDER_H
#define ESR_RENDER_H

#include <optional>
#include <unordered_map>
#include <utility>

#include "../app/plugin.h"
#include "../asset/asset_handle.h"
#include "../ecs/schedule.h"
#include "../geometry/mat3.h"
#include "../geometry/quat.h"
#include "../transform/components.h"
#include "mesh/mesh.h"
#include "render_api.h"
#include "render_phase/draw_state.h"

namespace esr {

struct MainGraphConstants {
    static const char* CAMERA_DRIVER;
};

class RenderPlugin : public Plugin {
    PLUGIN_INIT

    /// Initializes the renderer, sets up the [`RenderSet`](RenderSet) and creates the rendering sub-app.
    void build(App& app) override;
};

/// The simulation `World` of the application, stored as a resource.
/// This resource is only available during `ExtractSchedule` and not
/// during command application of that schedule.
/// See `Extract` for more details.
struct R_MainWorld {
    R_MainWorld(World* world) {
        world_ = world;
    }
    World* world_;
};

struct C_Visibility {
    bool visible = true;
};

// Should be used with a Handle.
struct Material {
    /// Returns this material's vertex shader. If [`ShaderRef::Default`] is returned, the default mesh vertex shader
    /// will be used.
    std::string vertex_shader() {
        return {};
    };

    /// Returns this material's fragment shader. If [`ShaderRef::Default`] is returned, the default mesh fragment shader
    /// will be used.
    std::string fragment_shader() {
        return {};
    };
};

/// A component bundle for entities with a `Mesh` and a `Material`.
struct MaterialMeshBundle {
    C_AssetHandle<Mesh<Vertex>> mesh;
    C_AssetHandle<Material> material;
    C_Transform transform;
    C_GlobalTransform global_transform;
    /// User indication of whether an entity is visible
    C_Visibility visibility;
};

/// Stores all uniforms of the component type.
struct R_ComponentUniforms {
    std::map<uint64_t, std::shared_ptr<int>> uniforms;
};

} // namespace esr

#endif // ESR_RENDER_H
