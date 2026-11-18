#include "app.h"

#include "../asset/asset.h"
#include "../core_pipeline/core_pipeline.h"
#include "../ecs/schedule.h"
#include "../input/input.h"
#include "../pbr/pbr.h"
#include "../render/render.h"
#include "../sprite/sprite.h"
#include "../text_3d/text_3d.h"
#include "../render/view/window.h"
#include "../transform/transform.h"
#include "../window/window.h"

namespace esr {

std::shared_ptr<App> App::from_empty() {
    auto app = std::shared_ptr<App>(new App);

    app->world = std::make_shared<World>();
    app->world->insert_resource(std::make_shared<R_Schedules>());

    app->main_schedule_label = std::make_shared<ScheduleLabelMain>();
    app->add_plugin(std::make_shared<MainSchedulePlugin>());

    // app->run_tests();

    return app;
}

std::shared_ptr<App> App::from_default() {
    auto app = from_empty();

    app->add_plugin(std::make_shared<WindowPlugin>());
    app->add_plugin(std::make_shared<RenderPlugin>());

    // Add these after adding the RenderPlugin.
    app->add_plugin(std::make_shared<CorePipelinePlugin>());
    app->add_plugin(std::make_shared<SpritePlugin>());
    app->add_plugin(std::make_shared<PbrPlugin>());

    app->add_plugin(std::make_shared<TransformPlugin>());

    app->add_plugin(std::make_shared<InputPlugin>());

    app->add_plugin(std::make_shared<Text3dPlugin>());

    return app;
}

std::shared_ptr<App> App::from_offscreen() {
    auto app = from_empty();

    app->add_plugin(std::make_shared<RenderPlugin>());

    // Add these after adding the RenderPlugin.
    app->add_plugin(std::make_shared<CorePipelinePlugin>());
    app->add_plugin(std::make_shared<SpritePlugin>());
    app->add_plugin(std::make_shared<PbrPlugin>());

    app->add_plugin(std::make_shared<TransformPlugin>());

    app->add_plugin(std::make_shared<Text3dPlugin>());

    return app;
}

void App::update() {
    world->try_run_schedule(main_schedule_label);

    // Update each sub app.
    for (auto &[label, sub_app] : sub_apps) {
        sub_app->extract(world.get());
        sub_app->run();

#ifndef ESR_NO_WINDOW
        auto windows_res = sub_app->app_->world->resource<std::shared_ptr<R_Windows>>();
        if (windows_res && windows_res->should_close) {
            should_close = true;
        }
#endif
        if (sub_app->app_->should_close) {
            should_close = true;
        }
    }
}

Result<int, AppError> App::add_plugin(std::shared_ptr<Plugin> plugin) {
    if (plugin->is_unique() && (plugin_name_added.find(plugin->name()) != plugin_name_added.end())) {
        std::cout << "Error: tried to add duplicate plugin: " + plugin->name() << std::endl;
        return {AppError::DuplicatePlugin};
    }

    plugin->build(*this);

    plugin_registry.push_back(plugin);

    std::cout << "Added plugin [" + plugin->name() << "]" << std::endl;

    return {0};
}

void App::add_startup_system(std::function<void(World &)> system) {
    std::cout << "Added startup system [" + std::string(system.target_type().name()) << "]" << std::endl;

    add_system(std::make_shared<ScheduleLabelStartup>(), system);
}

void App::add_system(const std::shared_ptr<ScheduleLabel> &schedule_label, std::function<void(World &)> system) {
    std::cout << "Added system [" << std::string(system.target_type().name())
              << "] to schedule [" + schedule_label->name() << "]" << std::endl;

    auto &schedules = world->resource_mut<std::shared_ptr<R_Schedules>>();

    auto res = schedules->get_mut(schedule_label);

    if (res.is_ok()) {
        res.unwrap()->add_system(system);
    } else {
        auto new_schedule = std::make_shared<Schedule>();
        new_schedule->add_system(system);
        schedules->insert(schedule_label, new_schedule);
    }
}

void App::insert_sub_app(std::string label, std::shared_ptr<SubApp> sub_app) {
    std::cout << "Added sub-app [" + label << "]" << std::endl;
    sub_apps[label] = sub_app;
}

void App::setup() {
    for (auto &plugin : plugin_registry) {
        plugin->setup(*this);
    }
}

void App::run() {
    setup();

    while (!should_close) {
        update();
    }
}

Result<std::shared_ptr<App>, GenericError> App::get_sub_app(std::string label) {
    auto iter = sub_apps.find(label);
    if (iter != sub_apps.end()) {
        return {iter->second->app_};
    }

    return {GenericError::Error};
}

void SubApp::run() {
    app_->update();
}

void SubApp::extract(World *main_world) {
    if (extract_) {
        extract_(main_world, *app_);
    }
}

} // namespace esr
