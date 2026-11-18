#ifndef ESR_RENDER_RESOURCE_PIPELINE_CACHE_H
#define ESR_RENDER_RESOURCE_PIPELINE_CACHE_H

#include <cstdint>
#include <map>
#include <memory>
#include <optional>

#include "../../utils/result.h"
#include "../render_api.h"

namespace esr {

/// Index of a cached render pipeline in a `PipelineCache`.
struct CachedRenderPipelineId : public OrderedId {};

/// Index of a cached shader resource descriptor set in a `ShaderResourceDescriptorSetCache`.
struct CachedShaderResourceDescriptorSetId : public OrderedId {};

/// Cache for render pipelines.
class R_RenderPipelineCache {
public:
    /// Try to retrieve a render pipeline GPU object from a cached ID.
    std::optional<std::shared_ptr<Pathfinder::RenderPipeline>> get_render_pipeline(CachedRenderPipelineId id) {
        auto iter = pipelines.find(id);
        if (iter == pipelines.end()) {
            return {};
        }
        return iter->second;
    }

    /// Insert a render pipeline into the cache.
    CachedRenderPipelineId add_render_pipeline(std::shared_ptr<Pathfinder::RenderPipeline> new_pipeline) {
        CachedRenderPipelineId id = {pipelines.size()};
        pipelines[id] = std::move(new_pipeline);
        return id;
    }

private:
    std::map<CachedRenderPipelineId, std::shared_ptr<Pathfinder::RenderPipeline>> pipelines;
};

class R_DescriptorSetCache {
public:
    std::optional<std::shared_ptr<Pathfinder::DescriptorSet>> get_descriptor_set(
        CachedShaderResourceDescriptorSetId id) {
        auto iter = sets.find(id);
        if (iter == sets.end()) {
            return {};
        }
        return iter->second;
    }

    CachedShaderResourceDescriptorSetId add_descriptor_Set(
        std::shared_ptr<Pathfinder::DescriptorSet> new_set) {
        CachedShaderResourceDescriptorSetId id = {sets.size()};
        sets[id] = std::move(new_set);
        return id;
    }

private:
    std::map<CachedShaderResourceDescriptorSetId, std::shared_ptr<Pathfinder::DescriptorSet>> sets;
};

} // namespace esr

#endif // ESR_RENDER_RESOURCE_PIPELINE_CACHE_H
