#include <iostream>
#include <stdexcept>

#include "../include/xysr.h"

using namespace esr;

struct C_MyModel {
    float timer = 0;
};

struct C_MySprite {
    float timer = 0;
};

void my_setup(World& world) {
    auto& assets = world.resource_mut<std::shared_ptr<R_Assets<Image>>>();

    AssimpLoader loader(&world);
    auto root_entity = loader.load("../assets/viking_room/scene.obj");

    if (root_entity.has_value()) {
        world.add_component(root_entity.value(), C_MyModel());
    }

    C_AssetHandle<Image> image_handle = assets->add(Image::from_empty({128, 128}));

    SpriteBundle bundle;
    bundle.texture = image_handle;
    bundle.transform.translation = {200, 200, 0};
    auto sprite_entity = bundle.spawn(world);
    world.add_component(sprite_entity, C_MySprite());

    Camera2dBundle camera2d_bundle;
    camera2d_bundle.camera.viewport = std::make_optional(Viewport{Vec2I(), Vec2I(1280, 720), {0.0, 1.0}});
    camera2d_bundle.camera_2d.clear_color = Color::white();
    camera2d_bundle.spawn(world);

    Camera3dBundle camera3d_bundle;
    camera3d_bundle.camera.viewport = std::make_optional(Viewport{Vec2I(), Vec2I(128, 128), {0.0, 1.0}});
    camera3d_bundle.transform.translation = Vec3F(1.0f);
    camera3d_bundle.transform.look_at(Vec3F(), Vec3F::Y());
    camera3d_bundle.camera.target = RenderTarget{{}, image_handle};
    camera3d_bundle.camera.order = -1;
    camera3d_bundle.spawn(world);
}

void my_update(World& world) {
    auto model_query = world.registry.view<C_Transform, C_MyModel>();

    for (auto [entity, transform, my_model] : model_query.each()) {
        my_model.timer += 0.01f;
        transform.rotation = Quat::from_rotation_y(my_model.timer);
    }

    auto sprite_query = world.registry.view<C_Transform, C_MySprite>();

    for (auto [entity, transform, my_sprite] : sprite_query.each()) {
        my_sprite.timer += 0.01f;
        transform.translation.x = 400 + 200 * sin(my_sprite.timer);
    }
}

class MyPlugin : public Plugin {
    PLUGIN_INIT

    void build(App& app) override {
        app.add_startup_system(my_setup);
        app.add_system(std::make_shared<ScheduleLabelUpdate>(), my_update);
    }
};

int main() {
    auto app = App::from_default();
    app->add_plugin(std::make_shared<MyPlugin>());

    app->run();

    return EXIT_SUCCESS;
}
