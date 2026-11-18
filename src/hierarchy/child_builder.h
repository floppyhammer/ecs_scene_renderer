#ifndef ESR_HIERARCHY_CHILD_BUILDER_H
#define ESR_HIERARCHY_CHILD_BUILDER_H

#include <optional>

#include "../ecs/world.h"
#include "components.h"

namespace esr {

/// Adds `child` to `parent`'s `Children`, and checking if it is already present there.
void push_child(World& world, Entity parent, Entity child);

/// Sets `Parent` of the `child` to `new_parent`.
///
/// Inserts `Parent` if `child` doesn't have one.
///
/// Returns the old parent if there's any.
std::optional<Entity> update_parent(World& world, Entity child, Entity new_parent);

/// Removes entities in `children` from `parent`'s `Children`, removing the component if it ends up empty.
/// Also removes `Parent` component from `children`.
void remove_children(World& world, Entity parent, std::vector<Entity> children_to_remove);

/// Removes all children from `parent` by removing its `Children` component, as well as removing
/// `Parent` component from its children.
void clear_children(World& world, Entity parent);

std::vector<Entity> get_descendents(World& world, Entity ancestor);

} // namespace esr

#endif // ESR_HIERARCHY_CHILD_BUILDER_H
