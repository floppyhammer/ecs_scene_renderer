#ifndef ESR_RENDER_PHASE_ITEM_H
#define ESR_RENDER_PHASE_ITEM_H

#include "../../ecs/world.h"
#include "../../geometry/basic.h"
#include "../../render/render_phase/draw.h"
#include "../render_resource/pipeline_cache.h"

namespace esr {

class DrawFunctionId;

class TrackedRenderPass;

class World;

/// The result of a batching operation.
enum class BatchResult {
    /// The `other` item was batched into `self`.
    Success,
    /// `self` and `other` cannot be batched together.
    IncompatibleItems,
};

/// An item (entity of the render world) which will be drawn to a texture or the screen,
/// as part of a `RenderPhase`.
///
/// The data required for rendering an entity is extracted from the main world in the `ExtractSchedule`.
/// Then it has to be queued up for rendering during the `QueueSchedule`,
/// by adding a corresponding phase item to a render phase.
/// Afterwards it will be sorted and rendered automatically in the
/// `PhaseSortSchedule` and `RenderSchedule`, respectively.
class PhaseItem {
public:
    virtual ~PhaseItem() = default;

    /// The corresponding entity that will be drawn. (This is not a view entity!)
    ///
    /// This is used to fetch the render data of the entity, required by the draw function,
    /// from the render world .
    Entity entity() const {
        return entity_;
    }

    /// Specifies the `Draw` function used to render the item.
    DrawFunctionId draw_function() {
        return draw_function_;
    }

    /// Sorts a slice of phase items into render order. Generally if the same type
    /// implements [`BatchedPhaseItem`], this should use a stable sort like [`slice::sort_by_key`].
    /// In almost all other cases, this should not be altered from the default,
    /// which uses a unstable sort, as this provides the best balance of CPU and GPU
    /// performance.
    ///
    /// Implementers can optionally not sort the list at all. This is generally advisable if and
    /// only if the renderer supports a depth prepass, which is by default not supported by
    /// the rest of Bevy's first party rendering crates. Even then, this may have a negative
    /// impact on GPU-side performance due to overdraw.
    ///
    /// It's advised to always profile for performance changes when changing this implementation.
    virtual void sort(const std::vector<std::shared_ptr<PhaseItem>> &items) = 0;

    CachedRenderPipelineId cached_pipeline() const {
        return pipeline_.value();
    }

    CachedShaderResourceDescriptorSetId cached_descriptor_set() const {
        return descriptor_set_.value();
    }

    /// Only for BatchedPhaseItem.
    std::reference_wrapper<const std::optional<Range>> batch_range() const {
        return std::cref(batch_range_);
    }

    /// Only for BatchedPhaseItem.
    std::reference_wrapper<std::optional<Range>> batch_range_mut() {
        return std::ref(batch_range_);
    }

    /// Only for BatchedPhaseItem.
    ///
    /// Batches another item within this item if they are compatible.
    /// Items can be batched together if they have the same entity, and consecutive ranges.
    /// If batching is successful, the `other` item should be discarded from the render pass.
    BatchResult add_to_batch(const PhaseItem &other);

protected:
    Entity entity_{};

    std::optional<CachedRenderPipelineId> pipeline_;

    std::optional<CachedShaderResourceDescriptorSetId> descriptor_set_;

    DrawFunctionId draw_function_{};

    /// Only available for batchable items.
    /// Range in the vertex buffer of this item.
    std::optional<Range> batch_range_;
};

} // namespace esr

#endif // ESR_RENDER_PHASE_ITEM_H
