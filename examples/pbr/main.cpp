#include <iostream>
#include <stdexcept>

#include "../include/xysr.h"

using namespace esr;

struct C_MyModel {
    float timer = 0;
};

void model_setup(World& world) {
    AssimpLoader loader(&world);
    auto root_entity = loader.load("../assets/models/viking_room.obj");

    if (root_entity.has_value()) {
        world.add_component(root_entity.value(), C_MyModel{});
    }

    auto floor_entity = loader.load("../assets/models/floor.obj");

    if (floor_entity.has_value()) {
        auto xform = C_Transform();
        xform.translation.y = -5;
        world.add_or_replace_component(floor_entity.value(), xform);
    }

    LightBundle light_bundle;
    light_bundle.transform.translation = Vec3F(0, 1.f, 0);
    light_bundle.spawn(world);

    Camera3dBundle camera_bundle;
    camera_bundle.camera.viewport = std::make_optional(Viewport{Vec2I(), Vec2I(1280, 720), {0.0, 1.0}});
    camera_bundle.transform.translation = Vec3F(1.2f);
    camera_bundle.transform.look_at(Vec3F(), Vec3F::Y());
    camera_bundle.spawn(world);
}

void model_update(World& world) {
    auto query = world.registry.view<C_Transform, C_MyModel>();

    auto input_server = world.resource_mut<std::shared_ptr<R_InputServer>>();
    auto& input_events = input_server->get_events();

    for (auto [entity, transform, my_model] : query.each()) {
        my_model.timer += 0.01f;
        transform.scale = Vec3F(0.5);

        for (auto& event : input_events) {
            if (event.type == InputEventType::MouseMotion) {
                transform.rotation = Quat::from_rotation_y(event.args.mouse_motion.position.x * 0.01);
            }
        }
    }
}

class RotatingModelPlugin : public Plugin {
    PLUGIN_INIT

    void build(App& app) override {
        app.add_startup_system(model_setup);
        app.add_system(std::make_shared<ScheduleLabelUpdate>(), model_update);
    }
};

int main() {
    auto app = App::from_default();
    app->add_plugin(std::make_shared<RotatingModelPlugin>());

    app->run();

    return EXIT_SUCCESS;
}
