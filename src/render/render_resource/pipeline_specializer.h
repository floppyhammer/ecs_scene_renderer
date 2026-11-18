#ifndef ESR_RENDER_RESOURCE_PIPELINE_SPECIALIZER_H
#define ESR_RENDER_RESOURCE_PIPELINE_SPECIALIZER_H

#include <bitset>
#include <map>
#include <utility>

#include "pipeline_cache.h"

namespace esr {

const uint32_t MAX_PIPELINE_KEY = 32;

typedef std::bitset<MAX_PIPELINE_KEY> PipelineKey;

/// Specialized pipelines used to retrieve the actual pipeline from cache.
template <typename P>
class R_SpecializedRenderPipelines {
public:
    CachedRenderPipelineId specialize(R_RenderPipelineCache& cache,
                                      std::shared_ptr<Pathfinder::RenderPipeline> specialize_pipeline,
                                      const PipelineKey& key) {
        auto id = cache.add_render_pipeline(std::move(specialize_pipeline));
        _cache[key.to_ulong()] = id;
        return id;
    }

    std::optional<CachedRenderPipelineId> find(const PipelineKey& key) {
        auto iter = _cache.find(key.to_ulong());
        if (iter != _cache.end()) {
            return iter->second;
        } else {
            return {};
        }
    }

private:
    std::map<unsigned long, CachedRenderPipelineId> _cache;
};

/// P stands for pipeline.
template <typename P, typename K>
class R_SpecializedDescriptorSets {
public:
    CachedShaderResourceDescriptorSetId specialize(
        R_DescriptorSetCache& cache,
        std::shared_ptr<Pathfinder::DescriptorSet> specialize_descriptor_set,
        const K& key) {
        auto id = cache.add_descriptor_Set(std::move(specialize_descriptor_set));
        _cache[key] = id;
        return id;
    }

    std::optional<CachedShaderResourceDescriptorSetId> find(const K& key) {
        auto iter = _cache.find(key);
        if (iter != _cache.end()) {
            return iter->second;
        } else {
            return {};
        }
    }

private:
    std::map<K, CachedShaderResourceDescriptorSetId> _cache;
};

} // namespace esr

#endif // ESR_RENDER_RESOURCE_PIPELINE_SPECIALIZER_H
