#ifndef ESR_RENDER_PHASE_DRAW_H
#define ESR_RENDER_PHASE_DRAW_H

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "../../ecs/entity.h"

namespace esr {

class World;

class PhaseItem;

class TrackedRenderPass;

/// The result of a `RenderCommand`.
enum class RenderCommandResult {
    Success,
    Failure,
};

class RenderCommand {
public:
    virtual ~RenderCommand() = default;

    /// Renders a `PhaseItem` by recording commands (e.g. setting pipelines, binding descriptors,
    /// issuing draw calls, etc.) via the `TrackedRenderPass`.
    virtual RenderCommandResult render(std::shared_ptr<PhaseItem> item, std::shared_ptr<TrackedRenderPass> pass) = 0;
};

/// A draw function used to draw `PhaseItem`s.
///
/// The draw function can retrieve and query the required ECS data from the render world.
template <typename P>
class Draw {
public:
    virtual ~Draw() = default;

    /// Prepares the draw function to be used. This is called once and only once before the phase
    /// begins. There may be zero or more `draw` calls following a call to this function.
    /// Implementing this is optional.
    virtual void prepare(const World& world) = 0;

    /// Draws a `PhaseItem` by issuing zero or more `draw` calls via the `TrackedRenderPass`.
    virtual void draw(const World& world, TrackedRenderPass& pass, Entity view_entity, P* item) = 0;
};

/// An identifier for a `Draw` function stored in `R_DrawFunctions`.
class DrawFunctionId {
public:
    uint64_t v;
};

/// Stores all `Draw` functions for the `PhaseItem` type.
///
/// For retrieval, the `Draw` functions are mapped to their respective `TypeId`s.
template <typename P>
struct R_DrawFunctions {
    R_DrawFunctions() {
        static_assert(std::is_base_of<PhaseItem, P>() && "DrawFunctions only accepts PhaseItem!");
    }

    std::vector<std::shared_ptr<Draw<P>>> draw_functions;
    std::map<std::string, DrawFunctionId> indices;

    /// Prepares all draw function. This is called once and only once before the phase begins.
    void prepare(const World& world) {
        for (auto& function : draw_functions) {
            function->prepare(world);
        }
    }

    /// Adds the `Draw` function and maps it to the type `T`
    template <typename T>
    DrawFunctionId add(std::shared_ptr<Draw<P>> draw_function) {
        auto id = DrawFunctionId{draw_functions.size()};
        draw_functions.push_back(draw_function);
        indices[typeid(T).name()] = id;
        return id;
    }

    /// Retrieves the `Draw` function corresponding to the `id` mutably.
    std::optional<std::shared_ptr<Draw<P>>> get_mut(DrawFunctionId id) {
        if (id.v < draw_functions.size()) {
            return {draw_functions[id.v]};
        }
        return {};
    }

    template <typename T>
    DrawFunctionId get_id() {
        return indices[typeid(T).name()];
    }
};

} // namespace esr

#endif // ESR_RENDER_PHASE_DRAW_H
