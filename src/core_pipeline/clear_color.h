#ifndef ESR_CORE_PIPELINE_CLEAR_COLOR_H
#define ESR_CORE_PIPELINE_CLEAR_COLOR_H

#include "../render/color.h"

namespace esr {

/// A `Resource` that stores the color that is used to clear the screen between frames.
///
/// This color appears as the "background" color for simple apps,
/// when there are portions of the screen with nothing rendered.
struct R_ClearColor {
    Color color;
};

} // namespace esr

#endif // ESR_CORE_PIPELINE_CLEAR_COLOR_H
