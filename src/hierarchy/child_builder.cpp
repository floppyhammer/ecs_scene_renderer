#include "child_builder.h"

#include <iostream>

namespace esr {

void push_child(World& world, Entity parent, Entity child) {
    bool has_children = world.registry.any_of<C_Children>(parent);

    if (has_children) {
        auto& children = world.registry.get<C_Children>(parent);

        if (std::find(children.entities.begin(), children.entities.end(), child) != children.entities.end()) {
            std::cout << "An entity is already a child of the parent!" << std::endl;
            return;
        }

        children.entities.push_back(child);
    } else {
        world.add_component(parent, C_Children{{child}});
    }

    world.add_component(child, C_Parent{parent});
}

std::optional<Entity> update_parent(World& world, Entity child, Entity new_parent) {
    bool has_parent = world.registry.any_of<C_Parent>(child);

    if (has_parent) {
        auto& parent = world.registry.get<C_Parent>(child);
        auto old_parent = parent.entity;
        parent.entity = new_parent;

        return old_parent;
    }
    return {};
}

void remove_children(World& world, Entity parent, std::vector<Entity> children_to_remove) {
    bool has_children = world.registry.any_of<C_Children>(parent);

    if (has_children) {
        auto& children = world.registry.get<C_Children>(parent);

        std::vector<Entity> remaining_children;
        for (auto& child : children.entities) {
            if (std::find(children_to_remove.begin(), children_to_remove.end(), child) == children_to_remove.end()) {
                remaining_children.push_back(child);
                world.registry.erase<C_Parent>(child);
            }
        }

        children.entities = remaining_children;

        if (remaining_children.empty()) {
            world.registry.erase<C_Children>(parent);
        }
    }
}

void clear_children(World& world, Entity parent) {
    bool has_children = world.registry.any_of<C_Children>(parent);

    if (!has_children) {
        return;
    }

    auto& children = world.registry.get<C_Children>(parent);

    // Erase Parent component from each child.
    for (auto& child : children.entities) {
        world.registry.erase<C_Parent>(child);
    }

    // Erase Children component from the parent.
    world.registry.erase<C_Children>(parent);
}

void propagate_descendents(World& world, Entity ancestor, std::vector<Entity>& descendents) {
    if (world.registry.any_of<C_Children>(ancestor)) {
        auto& children = world.registry.get<C_Children>(ancestor);

        for (auto& child : children.entities) {
            descendents.push_back(child);
        }

        for (auto& child : children.entities) {
            propagate_descendents(world, child, descendents);
        }
    }
}

std::vector<Entity> get_descendents(World& world, Entity ancestor) {
    std::vector<Entity> descendents;
    propagate_descendents(world, ancestor, descendents);
    return descendents;
}

} // namespace esr
