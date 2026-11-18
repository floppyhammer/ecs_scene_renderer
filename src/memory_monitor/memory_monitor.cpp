#include "memory_monitor.h"

#include "../app/app.h"
#include "../ecs/schedule.h"
#include "../ecs/world.h"

namespace esr {

void print_statistics(World& world) {
    auto assets = world.resource<std::shared_ptr<R_Assets<Mesh<Vertex>>>>();

    uint32_t asset_count = 0;
    uint32_t removed_asset_count = 0;
    uint32_t modified_asset_count = 0;

    for (auto& asset : assets->iter()) {
        asset_count += 1;

        if (asset.second.is_modified()) {
            modified_asset_count += 1;
        }

        //        if (asset.second.is_removed()) {
        //            removed_asset_count += 1;
        //        }
    }

    std::cout << "Asset count: " << asset_count << " | Modified asset count: " << modified_asset_count
              << " | Removed asset count: " << removed_asset_count;

    auto render_assets = world.resource<std::shared_ptr<R_Assets<Mesh<Vertex>>>>();
}

void MemoryMonitorPlugin::build(App& app) {
    app.add_system(std::make_shared<ScheduleLabelLast>(), print_statistics);
}

} // namespace esr
