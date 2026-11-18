#ifndef ESR_RENDER_VIEW_H
#define ESR_RENDER_VIEW_H

#include <optional>

#include "../../app/plugin.h"
#include "../../geometry/mat4.h"
#include "../../geometry/rect.h"
#include "../../transform/components.h"
#include "../render_phase/phase_item.h"
#include "../render_resource/render_resource.h"

namespace esr {

class App;

/// Handles views (render targets).
class ViewPlugin : public Plugin {
    PLUGIN_INIT

    void build(App& app) override;
};

/// Where to draw renderable things.
struct C_ViewTarget {
    std::shared_ptr<Pathfinder::Framebuffer> framebuffer;
#ifndef ESR_NO_WINDOW
    std::optional<std::shared_ptr<Pathfinder::SwapChain>> swapchain;
#endif
    Pathfinder::TextureFormat texture_format;
};

struct C_ExtractedView {
    Mat4 projection;
    /// Camera transform, which can be used to compute the view matrix.
    C_GlobalTransform transform;
    RectI viewport;
};

/// Shader uniform data.
struct ViewUniform {
    Mat4 view_proj;
    Mat4 view;
    Mat4 projection;
    Vec3F world_position;
    float pad0;
    RectF viewport;
};

/// The dynamic uniform buffer that stores all view uniforms.
struct R_ViewUniforms {
    DynamicUniformBuffer<ViewUniform> uniforms;
};

/// Offset in the dynamic uniform buffer.
struct C_ViewUniformOffset {
    uint32_t offset;
};

} // namespace esr

#endif // ESR_RENDER_VIEW_H
