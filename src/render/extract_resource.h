#ifndef ESR_RENDER_EXTRACT_RESOURCE_H
#define ESR_RENDER_EXTRACT_RESOURCE_H

#include "../app/app.h"
#include "../app/plugin.h"
#include "../render/render_schedule.h"

namespace esr {

/// This system extracts the resource of the corresponding [`Resource`] type.
template <typename R>
void extract_resource(World& world) {
    // , shared_ptr<R> main_resource, optional<shared_ptr<R>> target_resource

    //    if (target_resource.has_value()) {
    //        target_resource
    //            if main_resource
    //                .is_changed() {
    //                    *target_resource = R::extract_resource(&main_resource);
    //                }
    //        }
    //    else {
    //        commands.insert_resource(R::extract_resource(&main_resource));
    //    }
}

/// This plugin extracts the resources into the "render world".
///
/// Therefore it sets up the `ExtractSchedule` step for the specified `Resource`.
template <typename R>
class ExtractResourcePlugin : public Plugin {
    PLUGIN_INIT

    void build(App& app) override {
        auto res = app.get_sub_app("Render");

        if (res.is_ok()) {
            auto render_app = res.unwrap();
            render_app->add_system(std::make_shared<ScheduleLabelExtract>(), extract_resource<R>);
        }
    }
};

} // namespace esr

#endif // ESR_RENDER_EXTRACT_RESOURCE_H
