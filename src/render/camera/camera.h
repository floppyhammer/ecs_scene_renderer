#ifndef ESR_RENDER_CAMERA_H
#define ESR_RENDER_CAMERA_H

#include <optional>

#include "../../app/plugin.h"
#include "../../asset/asset_handle.h"
#include "../../geometry/basic.h"
#include "../../geometry/mat4.h"
#include "../../geometry/quat.h"
#include "../render_asset.h"
#include "../render_graph/node.h"
#include "../texture/image.h"

namespace esr {

struct R_ExtractedWindows;

class CameraPlugin : public Plugin {
    PLUGIN_INIT

    void build(App& app) override;
};

/// Configures the `RenderGraph` name assigned to be run for a given `Camera` entity.
struct C_CameraRenderGraph {
    std::string name;
};

/// Holds internally computed `Camera` values.
struct ComputedCameraValues {
    Mat4 projection_matrix;
    //    std::optional<RenderTargetInfo> target_info;
    std::optional<Vec2I> old_viewport_size;
};

/// Render viewport configuration for the `Camera` component.
struct Viewport {
    Vec2I physical_position;
    Vec2I physical_size;
    std::pair<float, float> depth;
};

class NormalizedRenderTarget;

enum class RenderTargetType {
    Window,
    Image,
};

/// The target that a `Camera` will render to.
struct RenderTarget {
    std::optional<Entity> window;
    std::optional<C_AssetHandle<Image>> image;

    std::optional<NormalizedRenderTarget> normalize(std::optional<Entity> primary_window);
};

/// The defining component for camera entities.
struct C_Camera {
    std::optional<Viewport> viewport;
    int32_t order = 0;
    bool is_active;
    ComputedCameraValues computed;
    RenderTarget target;
    bool msaa_writeback;

    Mat4 projection_matrix() const {
        return computed.projection_matrix;
    }
};

/// Normalized version of the render target.
class NormalizedRenderTarget {
public:
    RenderTargetType type;

    std::optional<Entity> window;
    std::optional<C_AssetHandleWeak<Image>> image;

    bool operator<(const NormalizedRenderTarget& b) const {
        if (type == b.type) {
            if (type == RenderTargetType::Window) {
                return window.value() < b.window.value();
            }

            return image.value() < b.image.value();
        }

        return type < b.type;
    }

    std::optional<Pathfinder::Texture*> get_texture_view(R_ExtractedWindows& windows,
                                                         R_RenderAssets<Image, GpuImage>& images);

    std::optional<Pathfinder::TextureFormat> get_texture_format(R_ExtractedWindows& windows,
                                                            R_RenderAssets<Image, GpuImage>& images);

    std::optional<std::shared_ptr<Pathfinder::Framebuffer>> get_framebuffer(R_ExtractedWindows& windows,
                                                                        R_RenderAssets<Image, GpuImage>& images);

    std::optional<std::shared_ptr<Pathfinder::SwapChain>> get_swapchain(R_ExtractedWindows& windows);
};

struct C_ExtractedCamera {
    std::optional<NormalizedRenderTarget> target;
    std::optional<Vec2I> physical_target_size;
    std::optional<Viewport> viewport;
    std::string render_graph;
    int32_t order;
    size_t sorted_camera_index_for_target;
};

struct SortedCamera {
    Entity entity;
    int32_t order;
    std::optional<NormalizedRenderTarget> target;
};

/// Cameras sorted by their order field.
class R_SortedCameras {
public:
    std::vector<SortedCamera> values;
};

class CameraDriverNode : public Node {
public:
    entt::runtime_view view;

    void update(World& world) override;

    Result<int, NodeRunError> run(RenderGraphContext& graph,
                                  RenderContext& render_context,
                                  const World& world) const override;
};

} // namespace esr

#endif // ESR_RENDER_CAMERA_H
