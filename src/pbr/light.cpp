#include "light.h"

#include "../ecs/world.h"

namespace esr {

void extract_lights(World& world) {
    auto main_world = world.resource<std::shared_ptr<R_MainWorld>>()->world_;

    auto& extracted_lights = world.resource_mut<std::shared_ptr<R_ExtractedLights>>();
    extracted_lights->point_lights.clear();

    auto query = main_world->registry.view<C_PointLight, C_GlobalTransform, C_Visibility>();

    for (auto [entity, point_light, transform, visibility] : query.each()) {
        if (!visibility.visible) {
            continue;
        }

        extracted_lights->point_lights.push_back({point_light, transform});
    }
}

void prepare_light_uniform(World& world) {
    auto& render_device = world.resource_mut<std::shared_ptr<R_RenderDevice>>();
    auto& render_queue = world.resource_mut<std::shared_ptr<R_RenderQueue>>();

    auto& light_uniform = world.resource_mut<std::shared_ptr<R_LightUniform>>();
    auto& extracted_lights = world.resource_mut<std::shared_ptr<R_ExtractedLights>>();

    if (!light_uniform->uniform) {
        light_uniform->uniform = std::make_shared<UniformBuffer<LightUniform>>();
    }

    LightUniform data;
    data.pointLightCount = std::min((uint32_t)extracted_lights->point_lights.size(), MAX_POINT_LIGHTS);

    for (uint32_t i = 0; i < data.pointLightCount; i++) {
        auto& light = extracted_lights->point_lights[i];
        data.pointLights[i].position = light.transform.translation;
        data.pointLights[i].ambient = light.light.color.to_vec().xyz();
    }

    light_uniform->uniform->set_value(data);

    light_uniform->uniform->write_buffer(*render_device, *render_queue);
}

void draw_shadow_map(World& world) {
    auto& render_device = world.resource_mut<std::shared_ptr<R_RenderDevice>>();

    Pathfinder::TextureDescriptor depth_texture_desc{
        Pathfinder::Vec2I{100, 100},
        Pathfinder::TextureFormat::Rgba8Unorm,
    };
    auto depth_texture = render_device->driver_->create_texture(depth_texture_desc, "shadow_map");

    auto& light_uniform = world.resource_mut<std::shared_ptr<R_LightUniform>>();
    light_uniform->shadow_map = depth_texture;

    auto render_pass = render_device->driver_->create_render_pass(depth_texture_desc.format, Pathfinder::AttachmentLoadOp::Clear, "shadow_pass");
    auto framebuffer = render_device->driver_->create_framebuffer(render_pass, depth_texture, "shadow_framebuffer");
}

Entity LightBundle::spawn(World& world) {
    auto entity = world.spawn();

    world.add_component(entity, light);
    world.add_component(entity, transform);
    world.add_component(entity, global_transform);
    world.add_component(entity, visibility);

    return entity;
}

} // namespace esr
