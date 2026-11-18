#include "context.h"

#include <iostream>

#include "../render_phase/draw_state.h"
#include "../view/view.h"
#include "graph.h"

namespace esr {

std::shared_ptr<TrackedRenderPass> RenderContext::begin_tracked_render_pass(const C_ViewTarget& target,
                                                                            bool depth_enabled,
                                                                            bool clear) {
    std::shared_ptr<Pathfinder::RenderPass> render_pass;
    std::shared_ptr<Pathfinder::CommandEncoder> command_encoder;
    std::shared_ptr<Pathfinder::Texture> target_texture;

    auto load_op = clear ? Pathfinder::AttachmentLoadOp::Clear : Pathfinder::AttachmentLoadOp::Load;

#ifndef ESR_NO_WINDOW
    if (target.swapchain.has_value()) {
        render_pass = render_device_->driver_->create_render_pass(target.texture_format, load_op, "SwapchainRenderPass");
        command_encoder = render_queue_->enqueue_swapchain_command_buffer(target.swapchain.value());
        target_texture = target.swapchain.value()->get_surface_texture();
    } else
#endif
    {
        render_pass = render_device_->driver_->create_render_pass(target.texture_format, load_op, "RenderPass");
        command_encoder = render_queue_->enqueue_command_buffer();
        if (target.framebuffer) {
            target_texture = target.framebuffer->get_texture();
        }
    }

    render_queue_->add_pass(render_pass);

    command_encoder->begin_render_pass(render_pass, target_texture, Pathfinder::ColorF{0.3f, 0.3f, 0.3f, 1.0f});

    auto tracked_render_pass = std::make_shared<TrackedRenderPass>(render_pass, target.framebuffer, command_encoder);

    return tracked_render_pass;
}

Result<int, RunSubGraphError> RenderGraphContext::run_sub_graph(const std::string& name,
                                                                std::vector<SlotValue> inputs,
                                                                std::optional<Entity> view_entity) {
    const auto& sub_graph = graph_.get_sub_graph(name);
    if (!sub_graph.has_value()) {
        std::cout << "RunSubGraphError::MissingSubGraph" << std::endl;
        return {RunSubGraphError::MissingSubGraph};
    }

    auto input_node = sub_graph.value().get().get_input_node();
    if (input_node.has_value()) {
        for (size_t i = 0; i < input_node.value().get().input_slots_.slots.size(); i++) {
            auto& input_slot = input_node.value().get().input_slots_.slots[i];
            if (i < inputs.size()) {
                auto& input_value = inputs[i];
                if (input_slot.slot_type != input_value.type) {
                    return {RunSubGraphError::MismatchedInputSlotType};
                }
                return {RunSubGraphError::MissingInput};
            }
        }
    } else if (!inputs.empty()) {
        return {RunSubGraphError::SubGraphHasNoInputs};
    }

    run_sub_graphs_.push_back(RunSubGraph{
        name,
        inputs,
        view_entity,
    });

    return {0};
}

std::vector<RunSubGraph> RenderGraphContext::finish() {
    auto value_to_return = run_sub_graphs_;
    run_sub_graphs_.clear();
    return value_to_return;
}

} // namespace esr
