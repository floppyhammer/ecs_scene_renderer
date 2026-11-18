#include <entt/entt.hpp>
#include <iostream>
#include <random>
#include <stdexcept>

#include "../include/xysr.h"

using namespace esr;

struct C_MySprite {
    float timer = 0;
};

void sprite_setup(World& world) {
    auto& assets = world.resource_mut<std::shared_ptr<R_Assets<Image>>>();

    std::optional<Entity> prev_entity;

    for (int i = 0; i < 3; i++) {
        SpriteBundle bundle;
        auto handle = assets->load("../assets/transparent_image.png").value();

        bundle.texture = handle;
        bundle.transform.translation.x = 100.0;
        bundle.transform.translation.y = 100.0;
        if (i == 0) {
            bundle.transform.translation.x = 400.0;
            bundle.transform.translation.y = 200.0;
        }

        auto entity = bundle.spawn(world);

        if (prev_entity.has_value()) {
            push_child(world, prev_entity.value(), entity);
        } else {
            world.add_component(entity, C_MySprite{});
        }
        prev_entity = entity;
    }

    Camera2dBundle camera_bundle;
    camera_bundle.camera.viewport = std::make_optional(Viewport{Vec2I(), Vec2I(1280, 720), {0.0, 1.0}});
    camera_bundle.spawn(world);
}

void sprite_update(World& world) {
    auto query = world.registry.view<C_Sprite, C_Transform, C_MySprite>();

    for (auto [entity, sprite, transform, my_sprite] : query.each()) {
        my_sprite.timer += 0.01f;
        transform.rotation = Quat::from_rotation_z(my_sprite.timer);
    }
}

class MovingSpritePlugin : public Plugin {
    PLUGIN_INIT

    void build(App& app) override {
        app.add_startup_system(sprite_setup);
        app.add_system(std::make_shared<ScheduleLabelUpdate>(), sprite_update);
    }
};

int main() {
    auto app = App::from_default();
    app->add_plugin(std::make_shared<MovingSpritePlugin>());

    app->run();

    return EXIT_SUCCESS;
}
