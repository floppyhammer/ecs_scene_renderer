#ifndef ESR_HIERARCHY_COMPONENTS_H
#define ESR_HIERARCHY_COMPONENTS_H

#include <vector>

namespace esr {

/// Contains references to the child entities of this entity.
struct C_Children {
    std::vector<Entity> entities;
};

/// Holds a reference to the parent entity of this entity.
/// This component should only be present on entities that actually have a parent entity.

struct C_Parent {
    Entity entity;
};

} // namespace esr

#endif // ESR_HIERARCHY_COMPONENTS_H
