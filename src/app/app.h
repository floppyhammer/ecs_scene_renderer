#ifndef ESR_APP_H
#define ESR_APP_H

#include <iostream>

#include "../ecs/schedule.h"
#include "../ecs/world.h"
#include "../render/render_api.h"
#include "../render/render_graph/graph.h"
#include "../utils/result.h"
#include "plugin.h"

namespace esr {

enum class AppError {
    DuplicatePlugin,
};

class SubApp;

/// A container of app logic and data.
///
/// Bundles together the necessary elements like [`World`] and [`Schedule`] to create
/// an ECS-based application.
class App {
public:
    /// Use static member functions below to construct an App.
    static std::shared_ptr<App> from_empty();

    static std::shared_ptr<App> from_default();

    static std::shared_ptr<App> from_offscreen();

    void run();

    /// Runs once.
    void setup();

    /// Runs repeatedly.
    void update();

    /// Adds a single `Plugin`.
    ///
    /// One of the core principles is modularity. All engine features are implemented
    /// as `Plugin`s. This includes internal features like the renderer.
    ///
    /// Never call this on a sub-app. Instead, add the plugin to the main app, then access the sub-app there.
    Result<int, AppError> add_plugin(std::shared_ptr<Plugin> plugin);

    /// Adds a system to the given schedule in this app's `Schedules`.
    void add_system(const std::shared_ptr<ScheduleLabel>& schedule_label, std::function<void(World&)> system);

    void add_startup_system(std::function<void(World&)> system);

    /// Inserts a [`Resource`] to the current [`App`] and overwrites any [`Resource`] previously added of the same type.
    ///
    /// A [`Resource`] represents globally unique data. [`Resource`]s must be added to apps
    /// before using them. This happens with [`insert_resource`](Self::insert_resource).
    template <typename T>
    void insert_resource(T resource) {
        world->insert_resource(resource);
    }

    /// Adds a new `schedule` to the `App` under the provided `label`.
    void add_schedule(std::shared_ptr<ScheduleLabel> label) {
        auto schedules_ = world->resource_mut<std::shared_ptr<R_Schedules>>();
        schedules_->insert(label, std::make_shared<Schedule>());
    }

    /// Inserts an existing sub app into the app
    void insert_sub_app(std::string label, std::shared_ptr<SubApp> sub_app);

    /// Retrieves a `SubApp` inside this [`App`] with the given label, if it exists. Otherwise returns
    /// an [`Err`] containing the given label.
    Result<std::shared_ptr<App>, GenericError> get_sub_app(std::string label);

    std::shared_ptr<ScheduleLabel> main_schedule_label;

    /// The main ECS `World` of the `App`.
    /// This stores and provides access to all the main data of the application.
    /// The systems of the `App` will run using this `World`.
    /// If additional separate `World`-`Schedule` pairs are needed, you can use
    /// `sub_app`(App::insert_sub_app)s.
    std::shared_ptr<World> world;

    bool should_close = false;

protected:
    App() = default;

    static void run_tests() {
        run_graph_tests();
    }

    std::vector<std::shared_ptr<Plugin>> plugin_registry;
    std::set<std::string> plugin_name_added;

    std::map<std::string, std::shared_ptr<SubApp>> sub_apps;
};

class SubApp {
public:
    SubApp(std::shared_ptr<App> app, std::function<void(World*, App&)> extract) : app_(app), extract_(extract) {
    }

    /// Runs the `SubApp`'s default schedule.
    void run();

    /// Extracts data from main world to this sub-app.
    void extract(World* main_world);

    /// The [`SubApp`]'s instance of [`App`].
    ///
    /// You shouldn't add plugins to a sub-app's app instance.
    std::shared_ptr<App> app_;

    /// A function that allows access to both the [`SubApp`] [`World`] and the main [`App`]. This is
    /// useful for moving data between the sub app and the main app.
    std::function<void(World*, App&)> extract_;
};

} // namespace esr

#endif // ESR_APP_H
