#ifndef ESR_HIERARCHY_H
#define ESR_HIERARCHY_H

#include <optional>

#include "../app/plugin.h"
#include "child_builder.h"

namespace esr {

class App;

/// The base plugin for handling `Parent` and `Children` components.
class HierarchyPlugin : public Plugin {
    PLUGIN_INIT

    void build(App& app) override;
};

//struct C_Relations {
//    std::size_t children{};
//    std::optional<Entity> first_child;
//    std::optional<Entity> prev_sibling;
//    std::optional<Entity> next_sibling;
//    std::optional<Entity> parent;
//};

std::vector<Entity> extract_entities_from_hierarchy(entt::registry& registry, Entity root);

} // namespace esr

#endif // ESR_HIERARCHY_H
