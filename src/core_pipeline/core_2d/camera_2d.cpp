#include "camera_2d.h"

#include "../../ecs/world.h"
#include "core_2d.h"

namespace esr {

Entity Camera2dBundle::spawn(World& world) {
    auto entity = world.spawn();

    world.add_component(entity, camera);
    world.add_component(entity, camera_render_graph);
    world.add_component(entity, projection);
    world.add_component(entity, transform);
    world.add_component(entity, global_transform);
    world.add_component(entity, camera_2d);

    return entity;
}

Camera2dBundle::Camera2dBundle() {
    camera.is_active = true;
    // This camera will use the core 2d render graph.
    camera_render_graph = {Core2dGraphConstants::CORE_2D};
    projection = C_Projection(OrthographicProjection());
}

} // namespace esr
