#include "render_schedule.h"

#include "../app/app.h"
#include "../ecs/world.h"

namespace esr {

void run_render_schedule(World& world) {
    auto& schedule_order = world.resource<std::shared_ptr<R_RenderScheduleOrder>>();

    for (const auto& label : schedule_order->get_labels()) {
        auto _ = world.try_run_schedule(label);
    }
}

} // namespace esr
