#include "world.h"

#include <iostream>

#include "../app/main_schedule.h"

namespace esr {

World::~World() {
    // Clear all entities.
    registry.clear();
}

Entity World::spawn() {
    return registry.create();
}

Entity World::get_or_spawn(Entity entity) {
    if (registry.valid(entity)) {
        return entity;
    }

    return registry.create(entity);
}

Result<int, TryRunScheduleError> World::try_run_schedule(std::shared_ptr<ScheduleLabel> label) {
    auto& schedules = resource_mut<std::shared_ptr<R_Schedules>>();

    auto res = schedules->find(label);
    if (!res.is_ok()) {
        std::cout << "Tried to run nonexistent schedule: " + label->name() << std::endl;
        return {TryRunScheduleError::Error};
    }

    res.unwrap()->run(*this);

    return {0};
}

} // namespace esr
