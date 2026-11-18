#include "core_pipeline.h"

#include "../app/app.h"
#include "../render/extract_resource.h"
#include "clear_color.h"
#include "core_2d/core_2d.h"
#include "core_3d/core_3d.h"

namespace esr {

void CorePipelinePlugin::build(App &app) {
    app.insert_resource(std::make_shared<R_ClearColor>());

    app.add_plugin(std::make_shared<ExtractResourcePlugin<R_ClearColor>>());
    app.add_plugin(std::make_shared<Core2dPlugin>());
    app.add_plugin(std::make_shared<Core3dPlugin>());
}

} // namespace esr
