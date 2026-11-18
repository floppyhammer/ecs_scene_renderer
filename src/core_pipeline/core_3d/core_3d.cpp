#include "core_3d.h"

#include "../../app/app.h"
#include "../../render/render_app.h"
#include "../../render/render_phase/render_phase.h"
#include "../../render/render_schedule.h"
#include "camera_3d.h"
#include "main_opaque_pass_3d_node.h"
#include "main_transparent_pass_3d_node.h"

namespace esr {

const char* Core3dGraphConstants::CORE_3D = "core_3d";

const char* Core3dGraphConstants::MSAA_WRITEBACK = "msaa_writeback";
const char* Core3dGraphConstants::PREPASS = "prepass";
const char* Core3dGraphConstants::START_MAIN_PASS = "start_main_pass";
const char* Core3dGraphConstants::MAIN_OPAQUE_PASS = "main_opaque_pass";
const char* Core3dGraphConstants::MAIN_TRANSPARENT_PASS = "main_transparent_pass";
const char* Core3dGraphConstants::END_MAIN_PASS = "end_main_pass";
const char* Core3dGraphConstants::BLOOM = "bloom";
const char* Core3dGraphConstants::TONEMAPPING = "tonemapping";
const char* Core3dGraphConstants::FXAA = "fxaa";
const char* Core3dGraphConstants::UPSCALING = "upscaling";
const char* Core3dGraphConstants::CONTRAST_ADAPTIVE_SHARPENING = "contrast_adaptive_sharpening";
const char* Core3dGraphConstants::END_MAIN_PASS_POST_PROCESSING = "end_main_pass_post_processing";

void Core3dPlugin::build(App& app) {
    auto res = app.get_sub_app("Render");
    if (!res.is_ok()) {
        return;
    }

    auto render_app = res.unwrap();

    render_app->insert_resource(std::make_shared<R_DrawFunctions<Opaque3d>>());
    render_app->insert_resource(std::make_shared<R_DrawFunctions<Transparent3d>>());

    render_app->add_system(std::make_shared<ScheduleLabelExtract>(), extract_core_3d_camera_phases);
    render_app->add_system(std::make_shared<ScheduleLabelPhaseSort>(), sort_phase_system<Opaque3d>);

    // Assemble core 3d sub-graph.
    {
        auto cast_render_app = (RenderApp*)render_app.get();

        cast_render_app->add_render_sub_graph(Core3dGraphConstants::CORE_3D);

        cast_render_app->add_render_graph_node<EmptyNode>(Core3dGraphConstants::CORE_3D, Core3dGraphConstants::PREPASS);
        cast_render_app->add_render_graph_node<EmptyNode>(Core3dGraphConstants::CORE_3D,
                                                          Core3dGraphConstants::START_MAIN_PASS);
        cast_render_app->add_render_graph_node<MainOpaquePass3dNode>(Core3dGraphConstants::CORE_3D,
                                                                     Core3dGraphConstants::MAIN_OPAQUE_PASS);
        cast_render_app->add_render_graph_node<MainTransparentPass3dNode>(Core3dGraphConstants::CORE_3D,
                                                                          Core3dGraphConstants::MAIN_TRANSPARENT_PASS);
        cast_render_app->add_render_graph_node<EmptyNode>(Core3dGraphConstants::CORE_3D,
                                                          Core3dGraphConstants::END_MAIN_PASS);
        cast_render_app->add_render_graph_node<EmptyNode>(Core3dGraphConstants::CORE_3D,
                                                          Core3dGraphConstants::TONEMAPPING);
        cast_render_app->add_render_graph_node<EmptyNode>(Core3dGraphConstants::CORE_3D,
                                                          Core3dGraphConstants::END_MAIN_PASS_POST_PROCESSING);
        cast_render_app->add_render_graph_node<EmptyNode>(Core3dGraphConstants::CORE_3D,
                                                          Core3dGraphConstants::UPSCALING);

        cast_render_app->add_render_graph_edges(Core3dGraphConstants::CORE_3D,
                                                {
                                                    Core3dGraphConstants::PREPASS,
                                                    Core3dGraphConstants::START_MAIN_PASS,
                                                    Core3dGraphConstants::MAIN_OPAQUE_PASS,
                                                    Core3dGraphConstants::MAIN_TRANSPARENT_PASS,
                                                    Core3dGraphConstants::END_MAIN_PASS,
                                                    Core3dGraphConstants::TONEMAPPING,
                                                    Core3dGraphConstants::END_MAIN_PASS_POST_PROCESSING,
                                                    Core3dGraphConstants::UPSCALING,
                                                });
    }
}

Opaque3d::Opaque3d(Entity entity,
                   CachedRenderPipelineId pipeline,
                   CachedShaderResourceDescriptorSetId descriptor_set,
                   DrawFunctionId draw_function,
                   float distance)
    : _distance(distance) {
    entity_ = entity;
    pipeline_ = pipeline;
    descriptor_set_ = descriptor_set;
    draw_function_ = draw_function;
}

Transparent3d::Transparent3d(Entity entity,
                             CachedRenderPipelineId pipeline,
                             CachedShaderResourceDescriptorSetId descriptor_set,
                             DrawFunctionId draw_function,
                             float distance)
    : _distance(distance) {
    entity_ = entity;
    pipeline_ = pipeline;
    descriptor_set_ = descriptor_set;
    draw_function_ = draw_function;
}

} // namespace esr
