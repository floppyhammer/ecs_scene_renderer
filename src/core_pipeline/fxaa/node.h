#ifndef ESR_CORE_PIPELINE_FXAA_NODE_H
#define ESR_CORE_PIPELINE_FXAA_NODE_H

#include "../../render/render_graph/node.h"

namespace esr {

class FxaaNode : public Node {
public:
    void update(World& world) override;

    Result<int, NodeRunError> run(RenderGraphContext& graph,
                                  RenderContext& render_context,
                                  const World& world) const override;
};

} // namespace esr
#endif // ESR_CORE_PIPELINE_FXAA_NODE_H
