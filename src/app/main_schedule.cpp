#include "main_schedule.h"

#include "../ecs/world.h"
#include "app.h"

namespace esr {

void MainSchedulePlugin::build(App& app) {
    app.insert_resource(std::make_shared<R_MainScheduleOrder>());

    app.add_schedule(std::make_shared<ScheduleLabelMain>());

    app.add_system(std::make_shared<ScheduleLabelMain>(), run_main_schedule);
}

void run_main_schedule(World& world) {
    static bool run_at_least_once = false;

    if (!run_at_least_once) {
        std::cout << "Main schedule startup (run once)" << std::endl;

        world.try_run_schedule(std::make_shared<ScheduleLabelPreStartup>());
        world.try_run_schedule(std::make_shared<ScheduleLabelStartup>());
        world.try_run_schedule(std::make_shared<ScheduleLabelPostStartup>());

        run_at_least_once = true;
    }

    auto& schedule_order = world.resource<std::shared_ptr<R_MainScheduleOrder>>();

    for (const auto& label : schedule_order->get_labels()) {
        auto _ = world.try_run_schedule(label);
    }
}

} // namespace esr
