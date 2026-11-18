#include "node.h"

#include "../../render/render_app.h"
#include "../../render/view/view.h"
#include "fxaa.h"

namespace esr {

void FxaaNode::update(World& world) {
    Node::update(world);
}

Result<int, NodeRunError> FxaaNode::run(RenderGraphContext& graph,
                                        RenderContext& render_context,
                                        const World& world) const {
    auto view_entity = graph.view_entity_;

    auto& pipeline_cache = world.resource<std::shared_ptr<R_RenderPipelineCache>>();
    auto& fxaa_pipeline = world.resource<R_FxaaPipeline>();

    bool has_fxaa = world.registry.all_of<C_ViewTarget, C_CameraFxaaPipeline, C_Fxaa>(view_entity.value());

    if (!has_fxaa) {
        return {0};
    }

    auto& target = world.registry.get<C_ViewTarget>(view_entity.value());
    auto& camera_fxaa_pipeline = world.registry.get<C_CameraFxaaPipeline>(view_entity.value());
    auto& fxaa = world.registry.get<C_Fxaa>(view_entity.value());

    if (!fxaa.enabled) {
        return {0};
    }

    auto pipeline = pipeline_cache->get_render_pipeline(camera_fxaa_pipeline.pipeline_id);
    if (pipeline.has_value()) {
        auto render_pass = render_context.begin_tracked_render_pass(target, false, false);

        //        render_pass.set_pipeline(pipeline);
        //        render_pass.set_bind_group(0, bind_group, &[]);
        //        render_pass->draw(0..3, 0..1);
    } else {
        abort();
    }

    return {0};
}

} // namespace esr
