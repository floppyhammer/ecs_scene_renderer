#ifndef ESR_MAIN_OPAQUE_PASS_3D_NODE_H
#define ESR_MAIN_OPAQUE_PASS_3D_NODE_H

#include "../../render/render_graph/node.h"

namespace esr {

class World;

class MainOpaquePass3dNode : public Node {
public:
    Result<int, NodeRunError> run(RenderGraphContext& graph,
                                  RenderContext& render_context,
                                  const World& world) const override;
};

} // namespace esr

#endif // ESR_MAIN_OPAQUE_PASS_3D_NODE_H
