#include "core_2d.h"

#include "../../app/app.h"
#include "../../ecs/world.h"
#include "../../render/render.h"
#include "../../render/render_app.h"
#include "../../render/render_phase/draw.h"
#include "../../render/render_phase/draw_state.h"
#include "../../render/render_phase/render_phase.h"
#include "../../render/render_resource/pipeline_cache.h"
#include "../../render/render_schedule.h"
#include "../../render/view/view.h"
#include "camera_2d.h"

namespace esr {

const char* Core2dGraphConstants::CORE_2D = "core_2d";

const char* Core2dGraphConstants::MSAA_WRITEBACK = "msaa_writeback";
const char* Core2dGraphConstants::MAIN_PASS = "main_pass";
const char* Core2dGraphConstants::BLOOM = "bloom";
const char* Core2dGraphConstants::TONEMAPPING = "tonemapping";
const char* Core2dGraphConstants::FXAA = "fxaa";
const char* Core2dGraphConstants::UPSCALING = "upscaling";
const char* Core2dGraphConstants::CONTRAST_ADAPTIVE_SHARPENING = "contrast_adaptive_sharpening";
const char* Core2dGraphConstants::END_MAIN_PASS_POST_PROCESSING = "end_main_pass_post_processing";

void extract_core_2d_camera_phases(World& world) {
    auto main_world = world.resource<std::shared_ptr<R_MainWorld>>()->world_;

    auto query = main_world->registry.view<C_Camera, C_Camera2d>();

    for (auto [main_world_entity, camera, camera_2d] : query.each()) {
        if (camera.is_active) {
            auto view_entity = world.get_or_spawn(main_world_entity);
            world.add_component(view_entity, C_RenderPhase<Transparent2d>());
            world.add_component(view_entity, camera_2d);
        }
    }
}

Result<int, NodeRunError> MainPass2dNode::run(RenderGraphContext& graph,
                                              RenderContext& render_context,
                                              const World& world) const {
    auto& view_entity = graph.view_entity_.value();

    bool has_required_components =
        world.registry
            .all_of<C_ExtractedCamera, C_RenderPhase<Transparent2d>, C_ViewTarget, C_Camera2d, C_ExtractedView>(
                view_entity);

    if (!has_required_components) {
        return {NodeRunError::UnknownError};
    }

    const auto [camera, transparent_phase, target, camera_2d, extracted_view] =
        world.registry.get<C_ExtractedCamera, C_RenderPhase<Transparent2d>, C_ViewTarget, C_Camera2d, C_ExtractedView>(
            view_entity);

    auto render_pass = render_context.begin_tracked_render_pass(target, false, true);

    if (camera.viewport.has_value()) {
        render_pass->set_camera_viewport(camera.viewport.value());
    } else {
        auto target_texture = target.framebuffer->get_texture();
        if (target_texture) {
            render_pass->set_viewport(
                RectI{0, 0, (int)target_texture->get_size().x, (int)target_texture->get_size().y}, 0.0, 1.0);
        }
    }

    transparent_phase.render(*render_pass, world, view_entity);

    render_pass->finish();

    return {0};
}

void MainPass2dNode::update(World& world) {}

void Core2dPlugin::build(App& app) {
    auto res = app.get_sub_app("Render");
    if (!res.is_ok()) {
        return;
    }

    auto render_app = res.unwrap();

    render_app->insert_resource(std::make_shared<R_DrawFunctions<Transparent2d>>());
    render_app->add_system(std::make_shared<ScheduleLabelExtract>(), extract_core_2d_camera_phases);
    render_app->add_system(std::make_shared<ScheduleLabelPhaseSort>(), sort_phase_system<Transparent2d>);
    render_app->add_system(std::make_shared<ScheduleLabelPhaseBatch>(), batch_phase_system<Transparent2d>);

    {
        auto cast_render_app = (RenderApp*)render_app.get();

        cast_render_app->add_render_sub_graph(Core2dGraphConstants::CORE_2D);

        cast_render_app->add_render_graph_node<MainPass2dNode>(Core2dGraphConstants::CORE_2D,
                                                               Core2dGraphConstants::MAIN_PASS);
        cast_render_app->add_render_graph_node<EmptyNode>(Core2dGraphConstants::CORE_2D,
                                                          Core2dGraphConstants::END_MAIN_PASS_POST_PROCESSING);

        cast_render_app->add_render_graph_edges(
            Core2dGraphConstants::CORE_2D,
            {Core2dGraphConstants::MAIN_PASS, Core2dGraphConstants::END_MAIN_PASS_POST_PROCESSING});
    }
}

Transparent2d::Transparent2d(Entity entity,
                             CachedRenderPipelineId pipeline,
                             CachedShaderResourceDescriptorSetId descriptor_set,
                             DrawFunctionId draw_function,
                             std::optional<Range> batch_range) {
    entity_ = entity;
    pipeline_ = pipeline;
    descriptor_set_ = descriptor_set;
    draw_function_ = draw_function;
    batch_range_ = batch_range;
}

} // namespace esr
