#include "fxaa.h"

#include "../../app/app.h"
#include "../../asset/asset.h"
#include "../../render/render_app.h"
#include "../../render/render_resource/pipeline_specializer.h"
#include "../../render/view/view.h"
#include "../../render/view/window.h"
#include "../core_2d/core_2d.h"
#include "../core_3d/core_3d.h"
#include "node.h"

namespace esr {

std::shared_ptr<Pathfinder::RenderPipeline> build_fxaa_pipeline(R_RenderDevice &device,
                                                            std::shared_ptr<Pathfinder::SwapChain> swap_chain) {
    auto driver = device.driver_;

    auto vert_code = load_file_as_bytes("../src/core_pipeline/fxaa/fxaa_vert.rsb").unwrap();
    auto frag_code = load_file_as_bytes("../src/core_pipeline/fxaa/fxaa_frag.rsb").unwrap();

    std::vector<char> vert_chars(vert_code.begin(), vert_code.end());
    std::vector<char> frag_chars(frag_code.begin(), frag_code.end());

    auto vert_shader_module = driver->create_shader_module(vert_chars, Pathfinder::ShaderStage::Vertex, "fxaa_vs");
    auto frag_shader_module = driver->create_shader_module(frag_chars, Pathfinder::ShaderStage::Fragment, "fxaa_fs");

    auto pipeline = driver->create_render_pipeline(
        vert_shader_module,
        frag_shader_module,
        {},
        Pathfinder::BlendState::from_over(),
        nullptr,
        swap_chain->get_surface_format(),
        "fxaa_pipeline");

    return pipeline;
}

struct FxaaPipelineKey {
    Sensitivity edge_threshold;
    Sensitivity edge_threshold_min;
    Pathfinder::TextureFormat texture_format;

    PipelineKey get_key() {
        auto pipeline_key = std::bitset<MAX_PIPELINE_KEY>();
        return pipeline_key;
    }
};

void prepare_fxaa_pipelines(World &world) {
    auto &render_device = world.resource_mut<std::shared_ptr<R_RenderDevice>>();
    auto &render_queue = world.resource_mut<std::shared_ptr<R_RenderQueue>>();
    auto driver = render_device->driver_;

    auto &windows = world.resource<std::shared_ptr<R_ExtractedWindows>>();
    auto &surfaces = world.resource<std::shared_ptr<R_WindowSurfaces>>();
    auto swap_chain = surfaces->surfaces[windows->primary.value()].swap_chain;

    auto &pipeline_cache = world.resource<std::shared_ptr<R_RenderPipelineCache>>();
    auto &descriptor_set_cache = world.resource<std::shared_ptr<R_DescriptorSetCache>>();
    auto &pipelines = world.resource_mut<std::shared_ptr<R_SpecializedRenderPipelines<R_FxaaPipeline>>>();

    auto query = world.registry.view<C_ExtractedView, C_Fxaa>();

    for (auto [entity, view, fxaa] : query.each()) {
        if (!fxaa.enabled) {
            continue;
        }

        auto pipeline_key = FxaaPipelineKey();
        pipeline_key = {fxaa.edge_threshold, fxaa.edge_threshold_min};

        auto pipeline_id = pipelines->find(pipeline_key.get_key());
        if (!pipeline_id.has_value()) {
            auto pipeline = build_fxaa_pipeline(*render_device, swap_chain);

            pipeline_id = pipelines->specialize(*pipeline_cache, pipeline, pipeline_key.get_key());
        }

        world.add_component(entity, C_CameraFxaaPipeline{pipeline_id.value()});
    }
}

void FxaaPlugin::build(App &app) {
    auto res = app.get_sub_app("Render");
    if (!res.is_ok()) {
        return;
    }

    const auto &render_app = (RenderApp *)res.unwrap().get();

    render_app->insert_resource(std::make_shared<R_SpecializedRenderPipelines<R_FxaaPipeline>>());
    render_app->insert_resource(
        std::make_shared<R_SpecializedDescriptorSets<R_FxaaPipeline, C_AssetHandle<Image>>>());

    render_app->add_render_graph_node<FxaaNode>(Core3dGraphConstants::CORE_3D, Core3dGraphConstants::FXAA);
    render_app->add_render_graph_edges(Core3dGraphConstants::CORE_3D,
                                       {Core3dGraphConstants::END_MAIN_PASS,
                                        Core3dGraphConstants::FXAA,
                                        Core3dGraphConstants::END_MAIN_PASS_POST_PROCESSING});

    render_app->add_render_graph_node<FxaaNode>(Core2dGraphConstants::CORE_2D, Core2dGraphConstants::FXAA);
    render_app->add_render_graph_edges(Core2dGraphConstants::CORE_2D,
                                       {Core2dGraphConstants::MAIN_PASS,
                                        Core2dGraphConstants::FXAA,
                                        Core2dGraphConstants::END_MAIN_PASS_POST_PROCESSING});
}

} // namespace esr
