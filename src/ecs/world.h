#ifndef ESR_ECS_WORLD_H
#define ESR_ECS_WORLD_H

#include <memory>

#include "../utils/result.h"
#include "entity.h"
#include "schedule.h"

namespace esr {

enum class TryRunScheduleError {
    Error,
};

class World {
public:
    ~World();

    Entity spawn();

    Entity get_or_spawn(Entity entity);

    template <typename Type, typename... Args>
    void emplace_component(const Entity entity, Args&&... args) {
        registry.emplace<Type>(entity, std::forward<Args>(args)...);
    }

    template <typename T>
    void add_component(const Entity entity, const T& component) {
        auto& c = registry.emplace<T>(entity);
        c = component;
    }

    template <typename T>
    void add_or_replace_component(const Entity entity, const T& component) {
        registry.emplace_or_replace<T>(entity, component);
    }

    /// Inserts a new resource with the given `value`.
    ///
    /// Resources are "unique" data of a given type.
    /// If you insert a resource of a type that already exists,
    /// you will overwrite any existing data.
    template <typename T>
    void insert_resource(const T& value) {
        static_assert(!std::is_pointer<T>() && "Expects resource of pointer type!");

        registry.ctx().insert_or_assign(value);
    }

    /// Same as resource(), for now.
    template <typename T>
    T& resource_mut() {
        return registry.ctx().get<T>();
    }

    /// Same as resource_mut(), for now.
    template <typename T>
    const T& resource() const {
        return registry.ctx().get<T>();
    }

    /// Attempts to run the `Schedule` associated with the `label` a single time,
    /// and returns a `TryRunScheduleError` if the schedule does not exist.
    Result<int, TryRunScheduleError> try_run_schedule(std::shared_ptr<ScheduleLabel> label);

    void queue_quit() {
        queued_quit = true;
    }

    bool should_quit() const {
        return queued_quit;
    }

    entt::registry registry;

private:
    entt::organizer render_organizer;

    bool queued_quit = false;
};

} // namespace esr

#endif // ESR_ECS_WORLD_H
