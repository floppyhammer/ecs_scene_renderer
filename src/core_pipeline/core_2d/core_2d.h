#ifndef ESR_RENDER_CORE_2D_H
#define ESR_RENDER_CORE_2D_H

#include "../../app/plugin.h"
#include "../../geometry/basic.h"
#include "../../render/camera/camera.h"
#include "../../render/render_api.h"
#include "../../render/render_graph/context.h"
#include "../../render/render_graph/node.h"
#include "../../render/render_phase/draw_state.h"
#include "../../render/render_phase/phase_item.h"

namespace esr {

class App;

class World;

class PhaseItem;

class DrawFunctionId;

struct CachedRenderPipelineId;

struct Core2dGraphConstants {
    // Subgraph name.
    static const char* CORE_2D;

    // Node names.
    static const char* MSAA_WRITEBACK;
    static const char* MAIN_PASS;
    static const char* BLOOM;
    static const char* TONEMAPPING;
    static const char* FXAA;
    static const char* UPSCALING;
    static const char* CONTRAST_ADAPTIVE_SHARPENING;
    static const char* END_MAIN_PASS_POST_PROCESSING;
};

class Core2dPlugin : public Plugin {
    PLUGIN_INIT

    void build(App& app) override;
};

class Transparent2d : public PhaseItem {
public:
    Transparent2d(Entity entity,
                  CachedRenderPipelineId pipeline,
                  CachedShaderResourceDescriptorSetId descriptor_set,
                  DrawFunctionId draw_function,
                  std::optional<Range> batch_range);

    void sort(const std::vector<std::shared_ptr<PhaseItem>>& items) override{
        // TODO
    };
};

class MainPass2dNode : public Node {
public:
    void update(World& world) override;

    Result<int, NodeRunError> run(RenderGraphContext& graph,
                                  RenderContext& render_context,
                                  const World& world) const override;
};

} // namespace esr

#endif // ESR_RENDER_CORE_2D_H
