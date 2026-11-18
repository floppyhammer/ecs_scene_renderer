#ifndef ESR_RENDER_PHASE_H
#define ESR_RENDER_PHASE_H

#include "phase_item.h"

namespace esr {

/// A collection of all rendering instructions, that will be executed by the GPU, for a
/// single render phase for a single view.
///
/// Each view (camera, or shadow-casting light, etc.) can have one or multiple render phases.
/// They are used to queue entities for rendering.
/// Multiple phases might be required due to different sorting/batching behaviors
/// (e.g. opaque: front to back, transparent: back to front) or because one phase depends on
/// the rendered texture of the previous phase (e.g. for screen-space reflections).
/// All `PhaseItem`s are then rendered using a single `TrackedRenderPass`.
/// The render pass might be reused for multiple phases to reduce GPU overhead.
template <typename I>
class C_RenderPhase {
public:
    std::vector<std::shared_ptr<PhaseItem>> items;

    /// Adds a `PhaseItem` to this render phase.
    void add(const std::shared_ptr<PhaseItem> &new_item) {
        items.push_back(new_item);
    }

    /// Sorts all of its `PhaseItem`s.
    void sort() {
        if (items.empty()) {
            return;
        }
    }

    /// Batches the compatible `BatchedPhaseItem`s of this render phase.
    void batch() {
        if (items.empty()) {
            return;
        }

        auto items_copy = items;
        items.clear();

        // Start the first batch from the first item.
        auto current_batch = items_copy.front();
        items_copy.erase(items_copy.begin());

        // Batch following items until we find an incompatible item.
        for (auto &next_item : items_copy) {
            if (current_batch->add_to_batch(*next_item) == BatchResult::IncompatibleItems) {
                // Store the completed batch, and start a new one from the incompatible item.
                items.push_back(current_batch);
                current_batch = next_item;
            }
        }
        // Store the last batch.
        items.push_back(current_batch);
    }

    /// Renders all of its `PhaseItem`s using their corresponding draw functions.
    void render(TrackedRenderPass &render_pass, const World &world, Entity view_entity) const {
        auto &draw_functions = world.resource<std::shared_ptr<R_DrawFunctions<I>>>();
        draw_functions->prepare(world);

        for (auto &item : items) {
            auto draw_function = draw_functions->get_mut(item->draw_function()).value();
            draw_function->draw(world, render_pass, view_entity, (I *)item.get());
        }
    }
};

/// This system sorts the `PhaseItem`s of all `RenderPhase`s of this type.
template <typename T>
void sort_phase_system(World &world) {
    auto query = world.registry.view<C_RenderPhase<T>>();

    for (auto [entity, phase] : query.each()) {
        phase.sort();
    }
}

/// This system batches the `PhaseItem`s of all `RenderPhase`s of this type.
template <typename T>
void batch_phase_system(World &world) {
    auto query = world.registry.view<C_RenderPhase<T>>();

    for (auto [entity, phase] : query.each()) {
        phase.batch();
    }
}

} // namespace esr

#endif // ESR_RENDER_PHASE_H
