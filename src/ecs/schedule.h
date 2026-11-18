#ifndef ESR_SCHEDULE_H
#define ESR_SCHEDULE_H

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "../utils/result.h"

namespace esr {

class World;

struct ScheduleLabel {
    virtual ~ScheduleLabel() = default;

    virtual std::string name() const = 0;
};

#define SCHEDULE_LABEL_INIT           \
    inline std::string name() const { \
        return typeid(this).name();   \
    }

class Schedule {
public:
    /// Runs all systems in this schedule on the `world`, using its current execution strategy.
    void run(World& world) const;

    /// Add a system to the schedule.
    void add_system(std::function<void(World&)> new_system);

private:
    /// Systems in the schedule will run in order.
    std::vector<std::function<void(World&)>> systems;
};

/// Resource that stores `Schedule`s mapped to `ScheduleLabel`s.
class R_Schedules {
    std::map<std::string, std::shared_ptr<Schedule>> inner;

public:
    Result<std::shared_ptr<Schedule>, GenericError> get_mut(std::shared_ptr<ScheduleLabel> label);

    void insert(std::shared_ptr<ScheduleLabel> label, std::shared_ptr<Schedule> schedule);

    Result<std::shared_ptr<Schedule>, GenericError> find(std::shared_ptr<ScheduleLabel> label);
};

/// Defines the schedules to be run, including their order.
class R_ScheduleOrder {
public:
    /// Adds the given `schedule` after the `after` schedule
    void insert_after(std::shared_ptr<ScheduleLabel> after, std::shared_ptr<ScheduleLabel> schedule) {
        std::optional<size_t> index;
        for (int i = 0; i < labels.size(); i++) {
            auto& label = labels[i];
            if (label->name() == after->name()) {
                index = std::make_optional(i);
            }
        }

        assert(index.has_value() && "Expected the AFTER schedule to exist!");

        labels.insert(labels.begin() + index.value() + 1, schedule);
    }

    const std::vector<std::shared_ptr<ScheduleLabel>>& get_labels() const {
        return labels;
    }

protected:
    /// The labels to run for the schedule (in the order they will be run).
    std::vector<std::shared_ptr<ScheduleLabel>> labels;
};

} // namespace esr

#endif // ESR_SCHEDULE_H
