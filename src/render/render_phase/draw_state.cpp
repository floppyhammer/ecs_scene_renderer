#include "draw_state.h"

#include "../camera/camera.h"

namespace esr {

void TrackedRenderPass::set_render_pipeline(CachedRenderPipelineId pipeline_id,
                                            const std::shared_ptr<Pathfinder::RenderPipeline>& pipeline) {
    if (state_->is_pipeline_set(pipeline_id)) {
        return;
    }
    command_encoder_->bind_render_pipeline(pipeline);
    state_->set_pipeline(pipeline_id);
}

void TrackedRenderPass::draw(Range vertices, Range instances) {
    if (instances.length() > 1) {
        command_encoder_->draw_instanced(vertices.length(), instances.length());
    } else {
        command_encoder_->draw(vertices.start, vertices.length());
    }
}

void TrackedRenderPass::draw_indexed(Range vertices, Range indices, Range instances) {
    command_encoder_->draw_indexed(indices.length(), instances.length(), indices.start, 0);
}

void TrackedRenderPass::set_viewport(const RectI& region, float min_depth, float max_depth) {
    command_encoder_->set_viewport(Pathfinder::RectI{region.min_x(), region.min_y(), region.width(), region.height()});
}

void TrackedRenderPass::set_camera_viewport(const Viewport& viewport) {
    set_viewport(RectI(viewport.physical_position, viewport.physical_position + viewport.physical_size),
                 viewport.depth.first,
                 viewport.depth.second);
}

void TrackedRenderPass::set_descriptor(
    uint32_t index,
    ShaderResourceDescriptorId descriptor_id,
    const std::shared_ptr<Pathfinder::DescriptorSet>& descriptor_set) {
    command_encoder_->bind_descriptor_set(descriptor_set);
    state_->set_descriptor(index, descriptor_id);
}

void TrackedRenderPass::set_descriptor_set(
    CachedShaderResourceDescriptorSetId descriptor_id,
    const std::shared_ptr<Pathfinder::DescriptorSet>& descriptor_set) {
    command_encoder_->bind_descriptor_set(descriptor_set);
}

void TrackedRenderPass::set_vertex_buffer(uint32_t index,
                                          BufferId buffer_id,
                                          size_t offset,
                                          const std::shared_ptr<Pathfinder::Buffer>& buffer) {
    std::vector<std::pair<std::shared_ptr<Pathfinder::Buffer>, uint64_t>> vertex_buffers = {{buffer, offset}};
    command_encoder_->bind_vertex_buffers(vertex_buffers);
    state_->set_vertex_buffer(index, buffer_id, offset);
}

void TrackedRenderPass::set_index_buffer(BufferId buffer_id,
                                         size_t offset,
                                         Pathfinder::DataType index_format,
                                         const std::shared_ptr<Pathfinder::Buffer>& buffer) {
    command_encoder_->bind_index_buffer(buffer.get());
    state_->set_index_buffer(buffer_id, offset, index_format);
}

void TrackedRenderPass::set_blend_constant() {
    // TODO
}

void TrackedRenderPass::finish() {
    command_encoder_->end_render_pass();
}

} // namespace esr
