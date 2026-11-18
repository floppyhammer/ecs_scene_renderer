#ifndef ESR_MEMORY_MONITOR_H
#define ESR_MEMORY_MONITOR_H

#include "../app/plugin.h"
#include "../asset/asset.h"
#include "../ecs/bundle.h"
#include "../ecs/world.h"
#include "../pbr/material.h"
#include "../render/render.h"
#include "../render/texture/image.h"
#include "../transform/components.h"

namespace esr {

class MemoryMonitorPlugin : public Plugin {
    PLUGIN_INIT

    void build(App& app) override;
};

} // namespace esr

#endif // ESR_MEMORY_MONITOR_H
