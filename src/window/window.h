#ifndef ESR_WINDOW_H
#define ESR_WINDOW_H

#include <optional>

#include "../app/plugin.h"
#include "../geometry/vec2.h"

namespace esr {

/// This plugin is optional. In some case, we need to integrate an App into another application.
class WindowPlugin : public Plugin {
    PLUGIN_INIT

    void build(App& app) override;
};

struct WindowResolution {
    Vec2I physical_size;
    /// System parameter.
    float scale_factor = 1.0;
    /// Custom parameter.
    std::optional<float> scale_factor_override;
};

/// Define how a window will be created and how it will behave.
struct C_Window {
    /// Stores the title of the window.
    std::string title;
    /// What resolution the window should have.
    WindowResolution resolution;
};

/// Marker component for the window considered the primary window.
///
/// Currently this is assumed to only exist on 1 entity at a time.
struct C_PrimaryWindow {
    int _ = 0;
};

} // namespace esr

#endif // ESR_WINDOW_H
