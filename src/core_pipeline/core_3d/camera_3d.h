#ifndef ESR_CORE_3D_CAMERA_H
#define ESR_CORE_3D_CAMERA_H

#include <optional>

#include "../../app/plugin.h"
#include "../../asset/asset_handle.h"
#include "../../ecs/bundle.h"
#include "../../geometry/basic.h"
#include "../../geometry/mat4.h"
#include "../../geometry/quat.h"
#include "../../render/camera/camera.h"
#include "../../render/camera/projection.h"
#include "../../render/color.h"
#include "../../transform/components.h"

namespace esr {

enum class DepthLoadOp {
    Clear,
    Load,
};

struct C_Camera3d {
    Color clear_color;
    DepthLoadOp depth_load_op;
};

/// Extract renderable phase items from the main world.
void extract_core_3d_camera_phases(World& world);

class Camera3dBundle : public Bundle {
public:
    Camera3dBundle();

    Entity spawn(World& world) override;

    C_Camera camera;
    C_CameraRenderGraph camera_render_graph;
    C_Projection projection;
    C_Transform transform;
    C_GlobalTransform global_transform;
    C_Camera3d camera_3d;
};

} // namespace esr

#endif // ESR_CORE_3D_CAMERA_H
