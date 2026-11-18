#ifndef ECS_SCENE_RENDERER_PBR_SHADOW_H
#define ECS_SCENE_RENDERER_PBR_SHADOW_H

#include <vector>

namespace esr {

struct R_PointLightShadowMap {
    size_t size = 1024;
};

struct R_DirectionalLightShadowMap {
    size_t size = 2048;
};

/// Controls how cascaded shadow mapping works.
/// Prefer using [`CascadeShadowConfigBuilder`] to construct an instance.
struct C_CascadeShadowConfig {
    /// The (positive) distance to the far boundary of each cascade.
    std::vector<float> bounds;
    /// The proportion of overlap each cascade has with the previous cascade.
    float overlap_proportion;
    /// The (positive) distance to the near boundary of the first cascade.
    float minimum_distance;
};

} // namespace esr

#endif // ECS_SCENE_RENDERER_PBR_SHADOW_H
