#ifndef ESR_CORE_2D_CAMERA_H
#define ESR_CORE_2D_CAMERA_H

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

struct C_Camera2d {
    Color clear_color;
};

class Camera2dBundle : public Bundle {
public:
    Camera2dBundle();

    Entity spawn(World& world) override;

    C_Camera camera;
    C_CameraRenderGraph camera_render_graph;
    C_Projection projection;
    C_Transform transform;
    C_GlobalTransform global_transform;
    C_Camera2d camera_2d;
};

} // namespace esr

#endif // ESR_CORE_2D_CAMERA_H
