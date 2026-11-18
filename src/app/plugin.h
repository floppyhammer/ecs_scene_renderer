#ifndef ESR_APP_PLUGIN_H
#define ESR_APP_PLUGIN_H

#include <string>
#include <typeinfo>

namespace esr {

class App;

#define PLUGIN_INIT                            \
    inline std::string name() const override { \
        return typeid(this).name();            \
    }

/// A collection of Bevy app logic and configuration.
///
/// Plugins configure an `App`. When an `App` registers a plugin,
/// the plugin's `Plugin::build` function is run. By default, a plugin
/// can only be added once to an `App`.
///
/// If the plugin may need to be added twice or more, the function `is_unique()`
/// should be overridden to return `false`. Plugins are considered duplicate if they have the same
/// `name()`.
class Plugin {
public:
    virtual ~Plugin() = default;

    /// Configures the `App` to which this plugin is added.
    virtual void build(App& app) = 0;

    /// Runs after all plugins are built, but before the app runner is called.
    /// This can be useful if you have some resource that other plugins need during their build step,
    /// but after build you want to remove it and send it to another thread.
    virtual void setup(App& app) {
    }

    /// Configures a name for the `Plugin` which is primarily used for checking plugin.
    /// uniqueness and debugging.
    virtual std::string name() const = 0;

    /// If the plugin can be meaningfully instantiated several times in an `App`,
    /// override this method to return `false`.
    virtual bool is_unique() {
        return true;
    }
};

} // namespace esr

#endif // ESR_APP_PLUGIN_H
