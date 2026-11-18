#include "view.h"

#include "../../app/app.h"
#include "../camera/camera.h"
#include "../render_asset.h"
#include "../render_schedule.h"
#include "window.h"

namespace esr {

/// Prepare view uniforms using the extracted views.
void prepare_view_uniforms(World& world) {
    auto& view_uniforms = world.resource_mut<std::shared_ptr<R_ViewUniforms>>();
    view_uniforms->uniforms.clear();

    auto query = world.registry.view<C_ExtractedView>();

    for (auto [entity, view] : query.each()) {
        const auto& viewport = view.viewport;
        const auto& projection = view.projection;

        ViewUniform view_uniform;
        view_uniform.view = view.transform.compute_matrix();
        view_uniform.projection = projection;
        view_uniform.view_proj = projection * view_uniform.view.inverse();
        view_uniform.viewport = viewport.to_f32();

        uint32_t offset = view_uniforms->uniforms.push(view_uniform);

        world.add_component(entity, C_ViewUniformOffset{offset});
    }

    auto& render_device = world.resource_mut<std::shared_ptr<R_RenderDevice>>();
    auto& render_queue = world.resource_mut<std::shared_ptr<R_RenderQueue>>();

    view_uniforms->uniforms.write_buffer(*render_device, *render_queue);
}

void prepare_view_targets(World& world) {
#ifndef ESR_NO_WINDOW
    auto& windows = world.resource_mut<std::shared_ptr<R_ExtractedWindows>>();
#endif
    auto& images = world.resource_mut<std::shared_ptr<R_RenderAssets<Image, GpuImage>>>();

    auto query = world.registry.view<C_ExtractedCamera, C_ExtractedView>();

    for (auto [entity, camera, view] : query.each()) {
        if (camera.physical_target_size.has_value() && camera.target.has_value()) {
            auto target_size = camera.physical_target_size.value();
            auto target = camera.target.value();

            if (target.type == RenderTargetType::Window) {
#ifndef ESR_NO_WINDOW
                auto target_swapchain = target.get_swapchain(*windows);

                if (target_swapchain.has_value()) {
                    auto main_texture_format = target_swapchain.value()->get_surface_format();

                    world.add_component(
                        entity,
                        C_ViewTarget{nullptr, target_swapchain.value(), main_texture_format});
                }
#endif
            } else {
                auto target_framebuffer = target.get_framebuffer(*windows, *images);

                if (target_framebuffer.has_value()) {
                    auto texture = target_framebuffer.value()->get_texture();
                    auto main_texture_format =
                        texture ? texture->get_format() : Pathfinder::TextureFormat::Rgba8Unorm;

                    world.add_component(entity, C_ViewTarget{target_framebuffer.value(), {}, main_texture_format});
                }
            }
        }
    }
}

void ViewPlugin::build(App& app) {
    auto res = app.get_sub_app("Render");
    if (!res.is_ok()) {
        return;
    }

    const auto& render_app = res.unwrap();

    render_app->insert_resource(std::make_shared<R_ViewUniforms>());

    render_app->add_system(std::make_shared<ScheduleLabelPrepare>(), prepare_view_uniforms);
    render_app->add_system(std::make_shared<ScheduleLabelPrepareB>(), prepare_view_targets);
}

} // namespace esr
