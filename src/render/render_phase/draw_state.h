#ifndef ESR_RENDER_DRAW_STATE_H
#define ESR_RENDER_DRAW_STATE_H

#include <cassert>
#include <optional>
#include <utility>
#include <vector>

#include "../../geometry/basic.h"
#include "../../geometry/rect.h"
#include "../../utils/result.h"
#include "../render_api.h"
#include "../render_resource/pipeline_cache.h"

#undef DrawState

namespace esr {

const uint8_t MAX_ANISOTROPY = 16;
const uint32_t MAX_SHADER_RESOURCE_DESCRIPTORS = 8;
const uint32_t MAX_VERTEX_BUFFERS = 16;
const uint32_t MAX_COLOR_ATTACHMENTS = 8;
const uint32_t MAX_MIP_LEVELS = 16;

struct Viewport;

struct ShaderResourceDescriptorId : public OrderedId {};

struct BufferId : public OrderedId {};

class DrawState {
public:
    DrawState() {
        _shader_resource_descriptors =
            std::vector<std::pair<std::optional<ShaderResourceDescriptorId>, std::vector<uint32_t>>>(
                MAX_SHADER_RESOURCE_DESCRIPTORS);
        _vertex_buffers = std::vector<std::optional<std::pair<BufferId, size_t>>>(MAX_VERTEX_BUFFERS);
    }

    void set_pipeline(CachedRenderPipelineId pipeline) {
        _pipeline = std::make_optional(pipeline);
    }

    bool is_pipeline_set(CachedRenderPipelineId pipeline) {
        return _pipeline.has_value() && _pipeline.value() == pipeline;
    }

    void set_descriptor_set(CachedShaderResourceDescriptorSetId descriptor_set) {
        _shader_resource_descriptor_set = std::make_optional(descriptor_set);
    }

    bool is_descriptor_set_set(CachedShaderResourceDescriptorSetId descriptor_set) {
        return _shader_resource_descriptor_set.has_value() && _shader_resource_descriptor_set.value() == descriptor_set;
    }

    void set_descriptor(uint32_t index, ShaderResourceDescriptorId descriptor) {
        auto& d = _shader_resource_descriptors[index];
        d.first = std::make_optional(descriptor);
        d.second.clear();
    }

    bool is_descriptor_set(uint32_t index, ShaderResourceDescriptorId descriptor) {
        assert(index < _shader_resource_descriptors.size());

        if (_shader_resource_descriptors[index].first.has_value()) {
            return _shader_resource_descriptors[index].first.value() == descriptor;
        }

        return false;
    }

    void set_vertex_buffer(uint32_t index, BufferId buffer, size_t offset) {
        _vertex_buffers[index] = std::make_optional(std::make_pair(buffer, offset));
    }

    bool is_vertex_buffer_set(uint32_t index, BufferId buffer, size_t offset) {
        assert(index < _shader_resource_descriptors.size());

        if (_vertex_buffers[index].has_value()) {
            return _vertex_buffers[index].value() == std::make_pair(buffer, offset);
        }

        return false;
    }

    void set_index_buffer(BufferId buffer, size_t offset, Pathfinder::DataType index_format) {
        _index_buffer = std::make_tuple(buffer, offset, index_format);
    }

    bool is_index_buffer_set(BufferId buffer, size_t offset, Pathfinder::DataType index_format) {
        return _index_buffer == std::make_tuple(buffer, offset, index_format);
    }

private:
    std::optional<CachedRenderPipelineId> _pipeline;
    std::optional<CachedShaderResourceDescriptorSetId> _shader_resource_descriptor_set;
    std::vector<std::pair<std::optional<ShaderResourceDescriptorId>, std::vector<uint32_t>>>
        _shader_resource_descriptors;
    std::vector<std::optional<std::pair<BufferId, size_t>>> _vertex_buffers;
    std::optional<std::tuple<BufferId, size_t, Pathfinder::DataType>> _index_buffer;
};

class TrackedRenderPass {
private:
    std::shared_ptr<Pathfinder::RenderPass> render_pass_;
    std::shared_ptr<Pathfinder::Framebuffer> framebuffer_;
    std::shared_ptr<Pathfinder::CommandEncoder> command_encoder_;
    std::shared_ptr<DrawState> state_;

public:
    TrackedRenderPass(std::shared_ptr<Pathfinder::RenderPass> render_pass,
                      std::shared_ptr<Pathfinder::Framebuffer> framebuffer,
                      std::shared_ptr<Pathfinder::CommandEncoder> command_encoder)
        : render_pass_(render_pass), framebuffer_(framebuffer), command_encoder_(command_encoder) {
        state_ = std::make_shared<DrawState>();
    }

    void set_render_pipeline(CachedRenderPipelineId pipeline_id,
                             const std::shared_ptr<Pathfinder::RenderPipeline>& pipeline);

    void set_descriptor(uint32_t index,
                        ShaderResourceDescriptorId descriptor_id,
                        const std::shared_ptr<Pathfinder::DescriptorSet>& descriptor_set);

    void set_descriptor_set(CachedShaderResourceDescriptorSetId descriptor_id,
                            const std::shared_ptr<Pathfinder::DescriptorSet>& descriptor_set);

    void set_vertex_buffer(uint32_t index,
                           BufferId buffer_id,
                           size_t offset,
                           const std::shared_ptr<Pathfinder::Buffer>& buffer);

    void set_index_buffer(BufferId buffer_id,
                          size_t offset,
                          Pathfinder::DataType index_format,
                          const std::shared_ptr<Pathfinder::Buffer>& buffer);

    void draw(Range vertices, Range instances);

    void draw_indexed(Range vertices, Range indices, Range instances);

    void set_viewport(const RectI& region, float min_depth, float max_depth);

    void set_camera_viewport(const Viewport& viewport);

    void set_blend_constant();

    void finish();
};

} // namespace esr

#endif // ESR_RENDER_DRAW_STATE_H
