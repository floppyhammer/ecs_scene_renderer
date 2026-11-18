#include "camera_3d.h"

#include "../../ecs/world.h"
#include "../../render/render_phase/render_phase.h"
#include "core_3d.h"

namespace esr {

void extract_core_3d_camera_phases(World& world) {
    auto main_world = world.resource<std::shared_ptr<R_MainWorld>>()->world_;

    auto query = main_world->registry.view<C_Camera, C_Camera3d>();

    for (auto [main_world_entity, camera, camera_3d] : query.each()) {
        if (camera.is_active) {
            // New entity in the render world, with the same ID as the main world entity.
            auto view_entity = world.get_or_spawn(main_world_entity);
            world.add_component(view_entity, C_RenderPhase<Opaque3d>());
            world.add_component(view_entity, C_RenderPhase<Transparent3d>());
            world.add_component(view_entity, camera_3d);
        }
    }
}

Entity Camera3dBundle::spawn(World& world) {
    auto entity = world.spawn();

    world.add_component(entity, camera);
    world.add_component(entity, camera_render_graph);
    world.add_component(entity, projection);
    world.add_component(entity, transform);
    world.add_component(entity, global_transform);
    world.add_component(entity, camera_3d);

    return entity;
}

Camera3dBundle::Camera3dBundle() {
    camera.is_active = true;
    // This camera will use the core 3d render graph.
    camera_render_graph = {Core3dGraphConstants::CORE_3D};
    projection = C_Projection(PerspectiveProjection());
}

} // namespace esr
