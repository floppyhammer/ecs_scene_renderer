#ifndef ESR_RENDER_API_H
#define ESR_RENDER_API_H

#include <memory>
#include <vector>

#include "pathfinder/gpu/buffer.h"
#include "pathfinder/gpu/command_encoder.h"
#include "pathfinder/gpu/descriptor_set.h"
#include "pathfinder/gpu/device.h"
#include "pathfinder/gpu/framebuffer.h"
#include "pathfinder/gpu/queue.h"
#include "pathfinder/gpu/render_pass.h"
#include "pathfinder/gpu/render_pipeline.h"
#include "pathfinder/gpu/swap_chain.h"
#include "pathfinder/gpu/texture.h"
#include "pathfinder/gpu/window.h"
#include "pathfinder/gpu/window_builder.h"

namespace esr {

class R_RenderDevice {
public:
    R_RenderDevice(std::shared_ptr<Pathfinder::Device> driver,
                   std::shared_ptr<Pathfinder::WindowBuilder> window_builder = nullptr)
        : driver_(driver), window_builder_(window_builder) {
    }

    std::shared_ptr<Pathfinder::Device> driver_;
    std::shared_ptr<Pathfinder::WindowBuilder> window_builder_;
};

class R_RenderQueue {
public:
    R_RenderQueue(std::shared_ptr<Pathfinder::Queue> queue, std::shared_ptr<Pathfinder::Device> driver)
        : queue_(queue), driver_(driver) {
    }

    std::shared_ptr<Pathfinder::CommandEncoder> enqueue_command_buffer() {
        auto encoder = driver_->create_command_encoder("CommandEncoder");

        command_encoders_.push_back(encoder);

        return encoder;
    }

    std::shared_ptr<Pathfinder::CommandEncoder> enqueue_swapchain_command_buffer(
        std::shared_ptr<Pathfinder::SwapChain> swapchain) {
        if (!swapchain_command_encoder_) {
            swapchain_command_encoder_ = driver_->create_command_encoder("SwapChainEncoder");
        }

        swapchain_ = swapchain;

        return swapchain_command_encoder_;
    }

    void submit() {
        for (size_t i = 0; i < command_encoders_.size(); i++) {
            auto &encoder = command_encoders_[i];

            queue_->submit(encoder, nullptr);
        }

        if (swapchain_command_encoder_ && swapchain_) {
            swapchain_->submit(swapchain_command_encoder_);
        }

        clear();
    }

    void clear() {
        command_encoders_.clear();

        render_passes_.clear();

        swapchain_command_encoder_.reset();
        swapchain_.reset();
    }

    std::shared_ptr<Pathfinder::Queue> get_queue_handle() {
        return queue_;
    }

    void add_pass(std::shared_ptr<Pathfinder::RenderPass> pass) {
        render_passes_.push_back(pass);
    }

private:
    std::shared_ptr<Pathfinder::Queue> queue_;
    std::shared_ptr<Pathfinder::Device> driver_;

    std::vector<std::shared_ptr<Pathfinder::CommandEncoder>> command_encoders_;

    std::shared_ptr<Pathfinder::CommandEncoder> swapchain_command_encoder_;
    std::shared_ptr<Pathfinder::SwapChain> swapchain_;

    // Hold the begun render passes until the commands are flushed.
    std::vector<std::shared_ptr<Pathfinder::RenderPass>> render_passes_;
};

} // namespace esr

#endif // ESR_RENDER_API_H
