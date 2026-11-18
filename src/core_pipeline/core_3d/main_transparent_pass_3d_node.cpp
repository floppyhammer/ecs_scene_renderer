#include "main_transparent_pass_3d_node.h"

#include "../../ecs/world.h"
#include "../../render/camera/camera.h"
#include "../../render/render.h"
#include "../../render/render_app.h"
#include "../../render/render_graph/context.h"
#include "../../render/render_graph/node.h"
#include "../../render/render_phase/draw.h"
#include "../../render/render_phase/draw_state.h"
#include "../../render/render_phase/render_phase.h"
#include "../../render/render_resource/pipeline_cache.h"
#include "../../render/render_schedule.h"
#include "../../render/view/view.h"
#include "camera_3d.h"
#include "core_3d.h"

namespace esr {

Result<int, NodeRunError> MainTransparentPass3dNode::run(RenderGraphContext& graph,
                                                         RenderContext& render_context,
                                                         const World& world) const {
    auto& view_entity = graph.view_entity_.value();

    bool has_required_components =
        world.registry
            .all_of<C_ExtractedCamera, C_RenderPhase<Transparent3d>, C_ViewTarget, C_Camera3d, C_ExtractedView>(
                view_entity);

    if (!has_required_components) {
        abort();
    }

    const auto [camera, opaque_phase, target, camera_3d, extracted_view] =
        world.registry.get<C_ExtractedCamera, C_RenderPhase<Transparent3d>, C_ViewTarget, C_Camera3d, C_ExtractedView>(
            view_entity);

    auto render_pass = render_context.begin_tracked_render_pass(target, true, false);

    if (camera.viewport.has_value()) {
        render_pass->set_camera_viewport(camera.viewport.value());
    } else {
        auto target_texture = target.framebuffer->get_texture();
        if (target_texture) {
            render_pass->set_viewport(
                RectI{0, 0, (int)target_texture->get_size().x, (int)target_texture->get_size().y}, 0.0, 1.0);
        }
    }

    opaque_phase.render(*render_pass, world, view_entity);

    render_pass->finish();

    return {0};
}

} // namespace esr
