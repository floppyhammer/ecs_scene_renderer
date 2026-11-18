#ifndef ESR_RENDER_GRAPH_NODE_H
#define ESR_RENDER_GRAPH_NODE_H

#include <cassert>
#include <utility>

#include "../../utils/result.h"
#include "../../utils/uuid.h"
#include "slot.h"

namespace esr {

struct RenderGraphContext;

struct RenderContext;

class World;

enum class NodeRunError {
    // Encountered an input slot error.
    InputSlotError,
    // Encountered an output slot error.
    OutputSlotError,
    // Encountered an error when running a sub-graph.
    RunSubGraphError,
    UnknownError,
};

struct NodeId {
    Uuid uuid;

    NodeId();

    bool operator<(const NodeId& rhs) const {
        return uuid < rhs.uuid;
    }

    bool operator==(const NodeId& rhs) const {
        return uuid == rhs.uuid;
    }
};

/// A [`NodeLabel`] is used to reference a [`NodeState`] by either its name or [`NodeId`]
/// inside the [`RenderGraph`](super::RenderGraph).
struct NodeLabel {
    enum class Type {
        Id,
        Name,
    } type;

    std::variant<NodeId, std::string> var;

    static NodeLabel from_id(NodeId id_) {
        return {Type::Id, id_};
    }

    static NodeLabel from_name(std::string name_) {
        return {Type::Name, std::move(name_)};
    }
};

/// A render node that can be added to a [`RenderGraph`](super::RenderGraph).
///
/// Nodes are the fundamental part of the graph and used to extend its functionality, by
/// generating draw calls and/or running subgraphs.
/// They are added via the `render_graph::add_node(my_node)` method.
///
/// To determine their position in the graph and ensure that all required dependencies (inputs)
/// are already executed, [`Edges`](Edge) are used.
///
/// A node can produce outputs used as dependencies by other nodes.
/// Those inputs and outputs are called slots and are the default way of passing render data
/// inside the graph. For more information see [`SlotType`](super::SlotType).
class Node {
public:
    virtual ~Node() = default;

    /// Specifies the required input slots for this node.
    /// They will then be available during the run method inside the [`RenderGraphContext`].
    virtual std::vector<SlotInfo> input() const;

    /// Specifies the produced output slots for this node.
    /// They can then be passed one inside [`RenderGraphContext`] during the run method.
    virtual std::vector<SlotInfo> output() const;

    /// Updates internal node state using the current render [`World`] prior to the run method.
    virtual void update(World& world);

    /// Runs the graph node logic, issues draw calls, updates the output slots and
    /// optionally queues up subgraphs for execution. The graph data, input and output values are
    /// passed via the [`RenderGraphContext`].
    virtual Result<int, NodeRunError> run(RenderGraphContext& graph,
                                          RenderContext& render_context,
                                          const World& world) const {
        return {0};
    }
};

/// A `Node` without any inputs, outputs and subgraphs, which does nothing when run.
/// Used (as a label) to bundle multiple dependencies into one inside
/// the `RenderGraph`.
class EmptyNode : public Node {};

} // namespace esr

#endif // ESR_RENDER_GRAPH_NODE_H
