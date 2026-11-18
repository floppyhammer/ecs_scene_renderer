#include "phase_item.h"

namespace esr {

BatchResult PhaseItem::add_to_batch(const PhaseItem &other) {
    auto self_entity = entity();

    auto &self_batch_range_opt = batch_range_mut().get();
    auto &other_batch_range_opt = other.batch_range().get();

    if (self_batch_range_opt.has_value() && other_batch_range_opt.has_value()) {
        auto &self_batch_range = self_batch_range_opt.value();
        auto &other_batch_range = other_batch_range_opt.value();

        // If the items are compatible, join their range into `self`.
        if (self_entity == other.entity()) {
            if (self_batch_range.end == other_batch_range.start) {
                self_batch_range.end = other_batch_range.end;
                return BatchResult::Success;
            }
            if (self_batch_range.start == other_batch_range.end) {
                self_batch_range.start = other_batch_range.start;
                return BatchResult::Success;
            }
        }
    }
    return BatchResult::IncompatibleItems;
}

} // namespace esr
