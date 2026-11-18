#include "node.h"

#include "../../ecs/world.h"

namespace esr {

NodeId::NodeId() {
    uuid = Uuid::from_random();
}

std::vector<SlotInfo> Node::input() const {
    return {};
}

std::vector<SlotInfo> Node::output() const {
    return {};
}

void Node::update(World& world) {
}

} // namespace esr
