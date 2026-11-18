#ifndef ESR_CORE_PIPELINE_FXAA_H
#define ESR_CORE_PIPELINE_FXAA_H

#include "../../app/plugin.h"
#include "../../render/render_resource/pipeline_cache.h"

namespace esr {

enum class Sensitivity {
    Low,
    Medium,
    High,
    Ultra,
    Extreme,
};

class FxaaPlugin : public Plugin {
    PLUGIN_INIT

    void build(App& app) override;
};

struct R_FxaaPipeline {
    int _;
};

struct C_CameraFxaaPipeline {
    CachedRenderPipelineId pipeline_id;
};

struct C_Fxaa {
    /// Enable render passes for FXAA.
    bool enabled;

    /// Use lower sensitivity for a sharper, faster, result.
    /// Use higher sensitivity for a slower, smoother, result.
    /// [Ultra](`Sensitivity::Ultra`) and [Extreme](`Sensitivity::Extreme`)
    /// settings can result in significant smearing and loss of detail.

    /// The minimum amount of local contrast required to apply algorithm.
    Sensitivity edge_threshold;

    /// Trims the algorithm from processing darks.
    Sensitivity edge_threshold_min;
};

} // namespace esr

#endif // ESR_CORE_PIPELINE_FXAA_H
