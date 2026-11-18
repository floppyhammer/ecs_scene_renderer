#ifndef ESR_RENDER_GRAPH_CONTEXT_H
#define ESR_RENDER_GRAPH_CONTEXT_H

#include "../render_api.h"
#include "node.h"
#include "slot.h"

namespace esr {

class RenderGraph;
class NodeState;
class TrackedRenderPass;
class C_ViewTarget;

struct RunSubGraph {
    std::string name;
    std::vector<SlotValue> inputs;
    std::optional<Entity> view_entity;
};

enum class RunSubGraphError {
    MissingSubGraph,
    SubGraphHasNoInputs,
    MissingInput,
    MismatchedInputSlotType,
};

struct RenderGraphContext {
    const RenderGraph& graph_;
    const NodeState& node_;
    const std::vector<SlotValue>& inputs_;
    std::vector<std::optional<SlotValue>>& outputs_;
    std::vector<RunSubGraph> run_sub_graphs_;
    std::optional<Entity> view_entity_;

    Result<int, RunSubGraphError> run_sub_graph(const std::string& name,
                                                std::vector<SlotValue> inputs,
                                                std::optional<Entity> view_entity);

    std::vector<RunSubGraph> finish();
};

struct RenderContext {
    R_RenderDevice* render_device_;
    R_RenderQueue* render_queue_;

    std::shared_ptr<Pathfinder::SwapChain> swapchain_;

    std::optional<std::shared_ptr<Pathfinder::Queue>> command_encoder_;
    std::vector<std::shared_ptr<Pathfinder::CommandEncoder>> command_buffers_;

    RenderContext(R_RenderDevice* render_device, R_RenderQueue* render_queue) {
        render_device_ = render_device;
        render_queue_ = render_queue;
    }

    std::shared_ptr<TrackedRenderPass> begin_tracked_render_pass(const C_ViewTarget& target,
                                                                 bool depth_enabled,
                                                                 bool clear);
};

enum class OutputSlotError {
    InvalidSlot,
    MismatchedSlotType,
};

enum class InputSlotError {
    InvalidSlot,
    MismatchedSlotType,
};

} // namespace esr

#endif // ESR_RENDER_GRAPH_CONTEXT_H
