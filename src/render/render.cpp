#include "render.h"

#include "../app/app.h"
#include "../ecs/world.h"
#include "camera/camera.h"
#include "render_api.h"
#include "render_app.h"
#include "render_asset.h"
#include "render_graph/graph.h"
#include "render_schedule.h"
#include "view/view.h"
#include "view/window.h"

namespace esr {

const char *MainGraphConstants::CAMERA_DRIVER = "camera_driver";

/// Updates the `RenderGraph` with all of its nodes and then runs it to render the entire frame.
void render_system(World &world) {
    auto render_device = world.resource<std::shared_ptr<R_RenderDevice>>();
    auto render_queue = world.resource<std::shared_ptr<R_RenderQueue>>();

#ifndef ESR_NO_WINDOW
    auto surfaces_res = world.resource<std::shared_ptr<R_WindowSurfaces>>();
    auto extracted_windows_res = world.resource<std::shared_ptr<R_ExtractedWindows>>();
    if (surfaces_res) {
        for (auto &[entity, surface] : surfaces_res->surfaces) {
            if (surface.swap_chain) {
                if (!surface.swap_chain->acquire_image()) {
                    return;
                }
                if (extracted_windows_res) {
                    auto &ext_win = extracted_windows_res->windows[entity];
                    auto render_pass = surface.swap_chain->get_render_pass();
                    auto surface_texture = surface.swap_chain->get_surface_texture();
                    if (surface_texture) {
                        ext_win.swapchain_framebuffer = render_device->driver_->create_framebuffer(
                            render_pass, surface_texture, "swapchain_framebuffer");
                    }
                }
            }
        }
    }
#endif

    // Advance frame for Pathfinder Frames-in-Flight (FIF) ring buffer & staging allocators
    render_device->driver_->begin_frame();
    render_queue->get_queue_handle()->begin_frame(render_device->driver_->get_current_frame_index());

    auto &graph = world.resource_mut<std::shared_ptr<RenderGraph>>();

    // Update graph.
    graph->update(world);

    // Run graph and generate command queue.
    auto res = RenderGraphRunner::run(*graph, render_device, render_queue, world);

    // Error running render graph.
    if (!res.is_ok()) {
        render_queue->clear();
        return;
    }

    render_queue->submit();
}

void reactive_update(entt::registry &registry) {
    // Observing matcher: an observer returns at least the entities for which
    // one or more of the given components have been updated and not yet destroyed.
    entt::observer observer{registry, entt::collector.update<C_Transform>()};

    for (const auto entity : observer) {
        // ...
    }

    observer.clear();
}

void y_sort(entt::registry &registry) {
    registry.sort<C_Transform>([](const auto &lhs, const auto &rhs) { return lhs.translation.z < rhs.translation.z; });
}

/// Executes the `ExtractSchedule` step of the renderer.
/// This updates the render world with the extracted ECS data of the current frame.
void extract(World *main_world, App &render_app) {
    // Temporarily add the app world to the render world as a resource.
    render_app.insert_resource(std::make_shared<R_MainWorld>(main_world));

    auto _ = render_app.world->try_run_schedule(std::make_shared<ScheduleLabelExtract>());
}

void clear_entities(World &world) {
    world.registry.clear();
}

void RenderPlugin::build(App &app) {
    // RENDER SUB-APP ---------------------------------

    auto render_app = RenderApp::from_empty();

    render_app->insert_resource(std::make_shared<R_RenderScheduleOrder>());
    render_app->insert_resource(std::make_shared<RenderGraph>());
    render_app->insert_resource(std::make_shared<R_RenderPipelineCache>());
    render_app->insert_resource(std::make_shared<R_DescriptorSetCache>());

    render_app->add_schedule(std::make_shared<ScheduleLabelExtract>());
    render_app->add_schedule(std::make_shared<ScheduleLabelMain>());

    render_app->add_system(std::make_shared<ScheduleLabelMain>(), run_render_schedule);
    render_app->add_system(std::make_shared<ScheduleLabelRender>(), render_system);
    render_app->add_system(std::make_shared<ScheduleLabelCleanup>(), clear_entities);
    render_app->add_system(std::make_shared<ScheduleLabelRenderFlush>(), flush_system);

    app.insert_sub_app("Render", std::make_shared<SubApp>(render_app, extract));

    app.add_plugin(std::make_shared<WindowRenderPlugin>());
    app.add_plugin(std::make_shared<CameraPlugin>());
    app.add_plugin(std::make_shared<ViewPlugin>());
    app.add_plugin(std::make_shared<RenderAssetPlugin<Image, GpuImage>>());
    app.add_plugin(std::make_shared<RenderAssetPlugin<Mesh<Vertex>, GpuMesh<Vertex>>>());
    app.add_plugin(std::make_shared<MeshPlugin>());

    app.add_plugin(std::make_shared<AssetPlugin<Image>>());
}

} // namespace esr
