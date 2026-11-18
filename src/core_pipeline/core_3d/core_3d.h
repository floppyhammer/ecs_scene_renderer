#ifndef ESR_RENDER_CORE_3D_H
#define ESR_RENDER_CORE_3D_H

#include "../../app/plugin.h"
#include "../../render/render_api.h"
#include "../../render/render_graph/context.h"
#include "../../render/render_graph/node.h"
#include "../../render/render_phase/draw_state.h"
#include "../../render/render_phase/phase_item.h"

namespace esr {

class App;

class World;

class DrawFunctionId;

struct CachedRenderPipelineId;

struct Core3dGraphConstants {
    // Subgraph name.
    static const char* CORE_3D;

    // Node names.
    static const char* MSAA_WRITEBACK;
    static const char* PREPASS;
    static const char* START_MAIN_PASS;
    static const char* MAIN_OPAQUE_PASS;
    static const char* MAIN_TRANSPARENT_PASS;
    static const char* END_MAIN_PASS;
    static const char* BLOOM;
    static const char* TONEMAPPING;
    static const char* FXAA;
    static const char* UPSCALING;
    static const char* CONTRAST_ADAPTIVE_SHARPENING;
    static const char* END_MAIN_PASS_POST_PROCESSING;
};

class Core3dPlugin : public Plugin {
    PLUGIN_INIT

    void build(App& app) override;
};

class Opaque3d : public PhaseItem {
public:
    Opaque3d(Entity entity,
             CachedRenderPipelineId pipeline,
             CachedShaderResourceDescriptorSetId descriptor_set,
             DrawFunctionId draw_function,
             float distance);

    void sort(const std::vector<std::shared_ptr<PhaseItem>>& items) override{
        // TODO
    };

private:
    float _distance;
};

class Transparent3d : public PhaseItem {
public:
    Transparent3d(Entity entity,
                    CachedRenderPipelineId pipeline,
                    CachedShaderResourceDescriptorSetId descriptor_set,
                    DrawFunctionId draw_function,
                    float distance);

    void sort(const std::vector<std::shared_ptr<PhaseItem>>& items) override{
        // TODO
    };

private:
    float _distance;
};

} // namespace esr

#endif // ESR_RENDER_CORE_3D_H
