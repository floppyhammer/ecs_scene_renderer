#ifndef ESR_APP_RENDER_SCHEDULE_H
#define ESR_APP_RENDER_SCHEDULE_H

#include <cassert>
#include <memory>
#include <optional>
#include <typeinfo>
#include <vector>

#include "../app/plugin.h"
#include "../ecs/schedule.h"

namespace esr {

class App;

class World;

/// Schedule which extract data from the main world and inserts it into the render world.
///
/// This step should be kept as short as possible to increase the "pipelining potential" for
/// running the next frame while rendering the current frame.
///
/// This schedule is run on the main world.
class ScheduleLabelExtract : public ScheduleLabel {
    SCHEDULE_LABEL_INIT
};

/// Prepare render resources from the extracted data for the GPU.
struct ScheduleLabelPrepare : public ScheduleLabel {
    SCHEDULE_LABEL_INIT
};

struct ScheduleLabelPrepareB : public ScheduleLabel {
    SCHEDULE_LABEL_INIT
};

/// Queue render commands.
struct ScheduleLabelQueue : public ScheduleLabel {
    SCHEDULE_LABEL_INIT
};

/// Sort the [`RenderPhases`](crate::render_phase::RenderPhase) here.
struct ScheduleLabelPhaseSort : public ScheduleLabel {
    SCHEDULE_LABEL_INIT
};

struct ScheduleLabelPhaseBatch : public ScheduleLabel {
    SCHEDULE_LABEL_INIT
};

/// Actual rendering happens here.
/// In most cases, only the render backend should insert resources here.
struct ScheduleLabelRender : public ScheduleLabel {
    SCHEDULE_LABEL_INIT
};

struct ScheduleLabelRenderFlush : public ScheduleLabel {
    SCHEDULE_LABEL_INIT
};

/// Cleanup render resources here.
struct ScheduleLabelCleanup : public ScheduleLabel {
    SCHEDULE_LABEL_INIT
};

class R_RenderScheduleOrder : public R_ScheduleOrder {
public:
    R_RenderScheduleOrder() {
        labels = {{
            std::make_shared<ScheduleLabelPrepare>(),
            std::make_shared<ScheduleLabelPrepareB>(),
            std::make_shared<ScheduleLabelQueue>(),
            std::make_shared<ScheduleLabelPhaseSort>(),
            std::make_shared<ScheduleLabelPhaseBatch>(),
            std::make_shared<ScheduleLabelRender>(),
            std::make_shared<ScheduleLabelRenderFlush>(),
            std::make_shared<ScheduleLabelCleanup>(),
        }};
    }
};

void run_render_schedule(World& world);

} // namespace esr

#endif // ESR_APP_RENDER_SCHEDULE_H
