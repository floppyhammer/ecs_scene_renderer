#ifndef ESR_RENDER_VIEW_WINDOW_H
#define ESR_RENDER_VIEW_WINDOW_H

#include <map>
#include <optional>
#include <set>

#include "../../app/plugin.h"
#include "../../ecs/entity.h"
#include "../../ecs/world.h"
#include "../../geometry/vec2.h"
#include "../render_api.h"

namespace esr {

#ifndef ESR_NO_WINDOW

/// An optional plugin used to render to windows.
class WindowRenderPlugin : public Plugin {
    PLUGIN_INIT

    void build(App& app) override;
};

struct ExtractedWindow {
    /// An main world entity that contains the `Window` component.
    Entity entity;

    Vec2I physical_size;

    std::optional<std::shared_ptr<Pathfinder::SwapChain>> swapchain_;
    std::optional<std::shared_ptr<Pathfinder::Framebuffer>> swapchain_framebuffer;
    std::optional<std::shared_ptr<Pathfinder::Texture>> swapchain_texture_view;
    std::optional<std::weak_ptr<Pathfinder::Texture>> swapchain_texture;
    std::optional<Pathfinder::TextureFormat> swapchain_texture_format;

    // If the window surface should be recreated.
    bool size_changed;
};

struct R_ExtractedWindows {
    /// The main world entity that contains the `PrimaryWindow` component.
    std::optional<Entity> primary;
    /// Main world entity with the `Window` component to render world window mapping.
    std::map<Entity, ExtractedWindow> windows;
};

struct SurfaceData {
    std::shared_ptr<Pathfinder::SwapChain> swap_chain;
    Pathfinder::TextureFormat format;
};

struct R_WindowSurfaces {
    /// Main world entity with the `Window` component to render world window mapping.
    std::map<Entity, SurfaceData> surfaces;
    /// List of windows that we have already called the initial `configure_surface` for.
    std::set<Entity> configured_windows;

    std::optional<SurfaceData> get(Entity entity) {
        auto iter = surfaces.find(entity);
        if (iter != surfaces.end()) {
            return surfaces[entity];
        }

        return {};
    }
};

class WrappedWindow {
public:
    WrappedWindow(GLFWwindow* handle) {
        handle_ = handle;
    }

    GLFWwindow* get_handle() const {
        return handle_;
    }

private:
    GLFWwindow* handle_{};
};

struct WindowId {
    GLFWwindow* v;

    inline bool operator<(const WindowId& b) const {
        return v < b.v;
    }
};

/// A resource which maps window entities to windows.
struct R_Windows {
    /// Stores windows by window identifier.
    std::map<WindowId, std::shared_ptr<WrappedWindow>> windows;
    /// Maps entities to window identifiers.
    std::map<Entity, WindowId> entity_to_window_id;
    /// Maps window identifiers to entities.
    std::map<WindowId, Entity> window_id_to_entity;

    bool should_close = false;

    std::optional<std::shared_ptr<WrappedWindow>> get_window_by_entity(Entity entity) {
        auto iter = entity_to_window_id.find(entity);

        if (iter != entity_to_window_id.end()) {
            return windows[iter->second];
        }

        return {};
    }
};

void flush_system(World& world);

#endif

} // namespace esr

#endif // ESR_RENDER_VIEW_WINDOW_H
