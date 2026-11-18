#ifndef ESR_RENDER_GRAPH_SLOT_H
#define ESR_RENDER_GRAPH_SLOT_H

#include <cassert>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "../../ecs/entity.h"
#include "../render_api.h"

namespace esr {

/// A `SlotLabel` is used to reference a slot by either its name or index
/// inside the `RenderGraph`.
struct SlotLabel {
    enum class Type {
        Index,
        Name,
    } type;

    std::variant<size_t, int, std::string> var;

    static SlotLabel from_index(size_t index) {
        return {
            Type::Index,
            index,
        };
    }

    static SlotLabel from_name(std::string name) {
        return {Type::Name, std::move(name)};
    }
};

enum class SlotType {
    Buffer,
    TextureView,
    Sampler,
    Entity,
};

struct SlotInfo {
    std::string name;
    SlotType slot_type;
};

struct SlotInfos {
    std::vector<SlotInfo> slots;

    std::optional<SlotInfo> get_slot(const SlotLabel& label) const {
        auto index = get_slot_index(label).value();
        assert(index < slots.size());
        return slots[index];
    }

    std::optional<size_t> get_slot_index(const SlotLabel& label) const {
        switch (label.type) {
            case SlotLabel::Type::Index:
                return std::make_optional(std::get<size_t>(label.var));
            case SlotLabel::Type::Name: {
                for (auto iter = slots.begin(); iter < slots.end(); ++iter) {
                    if (iter->name == std::get<std::string>(label.var)) {
                        return std::make_optional(iter - slots.begin());
                    }
                }
            }
        }

        return {};
    }
};

struct SlotValue {
    SlotType type;

    std::variant<std::shared_ptr<Pathfinder::Buffer>,
                 std::shared_ptr<Pathfinder::Texture>,
                 std::shared_ptr<Pathfinder::Sampler>,
                 Entity>
        var;
};

} // namespace esr

#endif // ESR_RENDER_GRAPH_SLOT_H
