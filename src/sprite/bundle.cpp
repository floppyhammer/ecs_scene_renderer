#include "bundle.h"

#include "../ecs/world.h"

namespace esr {

Entity SpriteBundle::spawn(World& world) {
    auto entity = world.spawn();
    world.add_component(entity, sprite);
    world.add_component(entity, transform);
    world.add_component(entity, global_transform);
    world.add_component(entity, texture);
    world.add_component(entity, visibility);

    return entity;
}

} // namespace esr
