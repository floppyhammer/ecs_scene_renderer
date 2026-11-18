#include "schedule.h"

#include "world.h"

namespace esr {

/// Runs all systems in this schedule on the `world`, using its current execution strategy.
void Schedule::run(World& world) const {
    for (auto& system : systems) {
        system(world);
    }
}

void Schedule::add_system(std::function<void(World&)> new_system) {
    systems.push_back(new_system);
}

Result<std::shared_ptr<Schedule>, GenericError> R_Schedules::get_mut(std::shared_ptr<ScheduleLabel> label) {
    auto iter = inner.find(label->name());

    if (iter != inner.end()) {
        return {iter->second};
    }
    return {GenericError::Error};
}

void R_Schedules::insert(std::shared_ptr<ScheduleLabel> label, std::shared_ptr<Schedule> schedule) {
    inner[label->name()] = schedule;
}

Result<std::shared_ptr<Schedule>, GenericError> R_Schedules::find(std::shared_ptr<ScheduleLabel> label) {
    const auto& iter = inner.find(label->name());
    if (iter == inner.end()) {
        return {GenericError::Error};
    }
    return {iter->second};
}

} // namespace esr
