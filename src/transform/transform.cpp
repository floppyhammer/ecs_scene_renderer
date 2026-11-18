#include "transform.h"

#include "../app/app.h"
#include "../app/main_schedule.h"
#include "../ecs/world.h"
#include "../hierarchy/components.h"
#include "../hierarchy/hierarchy.h"

namespace esr {

/// Update `GlobalTransform` component of entities that aren't in the hierarchy.
void sync_simple_transforms(World& world) {
    auto query = world.registry.view<C_Transform, C_GlobalTransform>(entt::exclude<C_Parent, C_Children>);

    for (auto [entity, transform, global_transform] : query.each()) {
        global_transform = C_GlobalTransform(transform);
    }
}

void traverse_children(World& world, Entity parent) {
    bool has_children = world.registry.any_of<C_Children>(parent);

    if (!has_children) {
        return;
    }

    auto& children = world.registry.get<C_Children>(parent);
    auto& parent_global_transform = world.registry.get<C_GlobalTransform>(parent);

    for (auto& child : children.entities) {
        auto& child_transform = world.registry.get<C_Transform>(child);
        auto& child_global_transform = world.registry.get<C_GlobalTransform>(child);

        auto local_transform = C_GlobalTransform(child_transform);
        child_global_transform = parent_global_transform * local_transform;

        traverse_children(world, child);
    }
}

/// Update `GlobalTransform` component of entities based on entity hierarchy and `Transform` component.
///
/// Third party plugins should ensure that this is used in concert with `sync_simple_transforms`.
void propagate_transforms(World& world) {
    // Find all orphaned entities with children. Transform propagation starts from these entities.
    auto query = world.registry.view<C_Transform, C_GlobalTransform, C_Children>(entt::exclude<C_Parent>);

    auto orphaned_entities = query.size_hint();

    for (auto [entity, transform, global_transform, children] : query.each()) {
        global_transform = C_GlobalTransform(transform);

        traverse_children(world, entity);
    }
}

void TransformPlugin::build(App& app) {
    app.add_system(std::make_shared<ScheduleLabelLast>(), sync_simple_transforms);
    app.add_system(std::make_shared<ScheduleLabelLast>(), propagate_transforms);
}

} // namespace esr
