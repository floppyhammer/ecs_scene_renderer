#include "camera.h"

#include <set>

#include "../../app/app.h"
#include "../../transform/components.h"
#include "../../window/window.h"
#include "../render.h"
#include "../render_schedule.h"
#include "../view/view.h"
#include "../view/window.h"
#include "projection.h"

namespace esr {

/// Extract cameras from the main world into the render world.
void extract_cameras(World& world) {
    auto main_world = world.resource<std::shared_ptr<R_MainWorld>>()->world_;

    // Get the main world entity that contains component `PrimaryWindow`.
    auto primary_window_query = main_world->registry.view<C_PrimaryWindow>();

    std::optional<Entity> primary_window_entity;
    for (auto [entity, _] : primary_window_query.each()) {
        primary_window_entity = {entity};
    }

    auto query = main_world->registry.view<C_Camera, C_CameraRenderGraph, C_GlobalTransform>();

    for (auto [main_world_entity, camera, graph, transform] : query.each()) {
        if (!camera.is_active) {
            continue;
        }

        if (camera.viewport.has_value()) {
            auto& viewport_origin = camera.viewport.value().physical_position;
            auto& viewport_size = camera.viewport.value().physical_size;

            auto& target_size = viewport_size;
            if (target_size.area() == 0) {
                continue;
            }

            auto view_entity = world.get_or_spawn(main_world_entity);

            auto normalized_target = camera.target.normalize(primary_window_entity);

            C_ExtractedCamera extracted_camera = {
                normalized_target.value(),
                target_size,
                camera.viewport,
                graph.name,
                camera.order,
                0,
            };
            world.add_component(view_entity, extracted_camera);

            C_ExtractedView extracted_view = {
                camera.projection_matrix(),
                transform,
                RectI(viewport_origin, viewport_size),
            };
            world.add_component(view_entity, extracted_view);
        }
    }
}

void sort_cameras(World& world) {
    auto& sorted_cameras = world.resource_mut<std::shared_ptr<R_SortedCameras>>();
    sorted_cameras->values.clear();

    auto query = world.registry.view<C_ExtractedCamera>();

    for (auto [entity, camera] : query.each()) {
        sorted_cameras->values.push_back(SortedCamera{
            entity,
            camera.order,
            camera.target,
        });
    }
}

void camera_system(World& world) {
    auto query = world.registry.view<C_Camera, C_Projection>();

    for (auto [entity, camera, camera_projection] : query.each()) {
        auto viewport_size = camera.viewport->physical_size;

        camera_projection.update(viewport_size.x, viewport_size.y);

        camera.computed.projection_matrix = camera_projection.get_projection_matrix();
    }
}

void CameraPlugin::build(App& app) {
    app.add_system(std::make_shared<ScheduleLabelPostUpdate>(), camera_system);

    auto res = app.get_sub_app("Render");

    if (res.is_ok()) {
        auto render_app = res.unwrap();

        render_app->insert_resource(std::make_shared<R_SortedCameras>());

        render_app->add_system(std::make_shared<ScheduleLabelExtract>(), extract_cameras);
        render_app->add_system(std::make_shared<ScheduleLabelPhaseSort>(), sort_cameras);

        auto camera_driver_node = std::make_shared<CameraDriverNode>();
        auto& render_graph = render_app->world->resource_mut<std::shared_ptr<RenderGraph>>();
        render_graph->add_node(MainGraphConstants::CAMERA_DRIVER, camera_driver_node);
    }
}

void CameraDriverNode::update(World& world) {
    // Do nothing.
}

Result<int, NodeRunError> CameraDriverNode::run(RenderGraphContext& graph_ctx,
                                                RenderContext& render_context,
                                                const World& world) const {
    const auto& sorted_cameras = world.resource<std::shared_ptr<R_SortedCameras>>();

    auto camera_windows = std::set<Entity>();

    for (auto& sorted_camera : sorted_cameras->values) {
        auto& camera = world.registry.get<C_ExtractedCamera>(sorted_camera.entity);

        if (camera.target.has_value() && camera.target->type == RenderTargetType::Window) {
            camera_windows.insert(camera.target->window.value());
        }

        auto res = graph_ctx.run_sub_graph(camera.render_graph, {}, std::make_optional(sorted_camera.entity));
        if (!res.is_ok()) {
            return {NodeRunError::RunSubGraphError};
        }
    }

    return {0};
}

std::optional<Pathfinder::Texture*> NormalizedRenderTarget::get_texture_view(
    R_ExtractedWindows& windows, R_RenderAssets<Image, GpuImage>& images) {
    if (type == RenderTargetType::Window) {
        auto iter = windows.windows.find(window.value());
        if (iter != windows.windows.end() && iter->second.swapchain_.has_value()) {
            return iter->second.swapchain_.value()->get_surface_texture().get();
        }
    } else {
        auto gpu_image = images.get_gpu_asset(image.value());
        if (gpu_image.has_value()) {
            return gpu_image.value()->texture_view;
        }
    }
    abort();
}

std::optional<Pathfinder::TextureFormat> NormalizedRenderTarget::get_texture_format(
    R_ExtractedWindows& windows, R_RenderAssets<Image, GpuImage>& images) {
    if (type == RenderTargetType::Window) {
        auto iter = windows.windows.find(window.value());
        if (iter != windows.windows.end()) {
            return iter->second.swapchain_texture_format;
        }
    } else {
        auto gpu_image = images.get_gpu_asset(image.value());
        if (gpu_image.has_value()) {
            return gpu_image.value()->texture_format;
        }
    }
    abort();
}

std::optional<std::shared_ptr<Pathfinder::Framebuffer>> NormalizedRenderTarget::get_framebuffer(
    R_ExtractedWindows& windows, R_RenderAssets<Image, GpuImage>& images) {
    if (type == RenderTargetType::Window) {
        auto iter = windows.windows.find(window.value());
        if (iter != windows.windows.end()) {
            return iter->second.swapchain_framebuffer;
        }
    } else {
        auto gpu_image = images.get_gpu_asset(image.value());

        if (gpu_image.has_value()) {
            return gpu_image.value()->framebuffer;
        }
    }
    abort();
}

std::optional<std::shared_ptr<Pathfinder::SwapChain>> NormalizedRenderTarget::get_swapchain(
    R_ExtractedWindows& windows) {
    if (type == RenderTargetType::Window) {
        auto iter = windows.windows.find(window.value());
        if (iter != windows.windows.end()) {
            return iter->second.swapchain_;
        }
    }
    return nullptr;
}

std::optional<NormalizedRenderTarget> RenderTarget::normalize(std::optional<Entity> primary_window) {
    if (window.has_value()) {
        return NormalizedRenderTarget{RenderTargetType::Window, window.value(), {}};
    }
    if (image.has_value()) {
        return NormalizedRenderTarget{RenderTargetType::Image, {}, image->as_weak()};
    }

    return NormalizedRenderTarget{RenderTargetType::Window, primary_window, {}};
}

} // namespace esr
