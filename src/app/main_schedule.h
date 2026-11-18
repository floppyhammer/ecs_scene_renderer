#ifndef ESR_APP_MAIN_SCHEDULE_H
#define ESR_APP_MAIN_SCHEDULE_H

#include <memory>
#include <optional>
#include <typeinfo>
#include <vector>

#include "../ecs/schedule.h"
#include "plugin.h"

namespace esr {

class App;

class World;

/// The schedule that contains the app logic that is evaluated each tick of `App::update()`.
///
/// By default, it will run the following schedules in the given order:
///
/// On the first run of the schedule (and only on the first run), it will run:
/// * `PreStartup`
/// * `Startup`
/// * `PostStartup`
///
/// Then it will run:
/// * `First`
/// * `PreUpdate`
/// * `Update`
/// * `PostUpdate`
/// * `Last`
struct ScheduleLabelMain : ScheduleLabel {
    SCHEDULE_LABEL_INIT
};

/// The schedule that runs before `Startup`.
/// This is run by the `Main` schedule.
struct ScheduleLabelPreStartup : ScheduleLabel {
    SCHEDULE_LABEL_INIT
};

/// The schedule that runs once when the app starts.
/// This is run by the `Main` schedule.
struct ScheduleLabelStartup : ScheduleLabel {
    SCHEDULE_LABEL_INIT
};

/// The schedule that runs once after `Startup`.
/// This is run by the `Main` schedule.
struct ScheduleLabelPostStartup : ScheduleLabel {
    SCHEDULE_LABEL_INIT
};

/// Runs first in the schedule.
/// This is run by the `Main` schedule.
struct ScheduleLabelFirst : ScheduleLabel {
    SCHEDULE_LABEL_INIT
};

/// The schedule that contains logic that must run before `Update`. For example, a system that reads raw keyboard
/// input OS events into an `Events` resource. This enables systems in `Update` to consume the events from the
/// `Events` resource without actually knowing about (or taking a direct scheduler dependency on) the "os-level keyboard
/// event system".
///
/// This is run by the `Main` schedule.
struct ScheduleLabelPreUpdate : ScheduleLabel {
    SCHEDULE_LABEL_INIT
};

/// The schedule that contains app logic.
/// This is run by the `Main` schedule.
struct ScheduleLabelUpdate : ScheduleLabel {
    SCHEDULE_LABEL_INIT
};

/// The schedule that contains logic that must run after `Update`. For example, synchronizing "local transforms" in a
/// hierarchy to "global" absolute transforms. This enables the `PostUpdate` transform-sync system to react to "local
/// transform" changes in
/// `Update` without the `Update` systems needing to know about (or add scheduler dependencies for) the "global
/// transform sync system".
///
/// This is run by the `Main` schedule.
struct ScheduleLabelPostUpdate : ScheduleLabel {
    SCHEDULE_LABEL_INIT
};

/// Runs last in the schedule.
/// This is run by the `Main` schedule.
struct ScheduleLabelLast : ScheduleLabel {
    SCHEDULE_LABEL_INIT
};

class R_MainScheduleOrder : public R_ScheduleOrder {
public:
    R_MainScheduleOrder() {
        labels = {{
            std::make_shared<ScheduleLabelFirst>(),
            std::make_shared<ScheduleLabelPreUpdate>(),
            std::make_shared<ScheduleLabelUpdate>(),
            std::make_shared<ScheduleLabelPostUpdate>(),
            std::make_shared<ScheduleLabelLast>(),
        }};
    }
};

/// A system that runs the "main schedule".
void run_main_schedule(World& world);

/// Initializes the `Main` schedule, sub schedules, and resources for a given `App`.
class MainSchedulePlugin : public Plugin {
    PLUGIN_INIT

    void build(App& app) override;
};

} // namespace esr

#endif // ESR_APP_MAIN_SCHEDULE_H
