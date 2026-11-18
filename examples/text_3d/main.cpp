#include <iostream>
#include <stdexcept>

#include "../include/xysr.h"

using namespace esr;

struct C_MyText {
    float timer = 0;
};

void text_setup(World& world) {
    auto& font_assets = world.resource_mut<std::shared_ptr<R_Assets<Text3dFont>>>();
    auto& image_assets = world.resource_mut<std::shared_ptr<R_Assets<Image>>>();
    auto& material_assets = world.resource_mut<std::shared_ptr<R_Assets<StandardMaterial>>>();

    auto font_handle = font_assets->load("../assets/fonts/NotoSans-Regular.ttf").value();
    auto color_texture = image_assets->load("../assets/opaque_image_2.jpg").value();

    // New material handle.
    auto material_ = std::make_shared<StandardMaterial>();
    material_->name = "text 3d";
    material_->set_base_color_texture(color_texture);
    auto material_handle = material_assets->add(material_);

    Text3dBundle text_bundle;
    text_bundle.font = font_handle;
    text_bundle.text = {"Hello world!"};
    text_bundle.material = material_handle;
    auto text_entity = text_bundle.spawn(world);
    world.add_component(text_entity, C_MyText{});

    LightBundle light_bundle;
    light_bundle.transform.translation = Vec3F(0, 0, 1.f);
    light_bundle.spawn(world);

    Camera3dBundle camera_bundle;
    camera_bundle.camera.viewport = std::make_optional(Viewport{Vec2I(), Vec2I(1280, 720), {0.0, 1.0}});
    camera_bundle.transform.translation = Vec3F(0, 0, 5.0f);
    camera_bundle.transform.look_at(Vec3F(0.0), Vec3F::Y());
    camera_bundle.spawn(world);
}

void text_update(World& world) {
    auto query = world.registry.view<C_Transform, C_MyText>();

    for (auto [entity, transform, my_text] : query.each()) {
        my_text.timer += 0.01f;

        transform.rotation = Quat::from_rotation_y(my_text.timer);
    }
}

class RotatingTextPlugin : public Plugin {
    PLUGIN_INIT

    void build(App& app) override {
        app.add_startup_system(text_setup);
        app.add_system(std::make_shared<ScheduleLabelUpdate>(), text_update);
    }
};

int main() {
    auto app = App::from_default();
    app->add_plugin(std::make_shared<RotatingTextPlugin>());

    app->run();

    return EXIT_SUCCESS;
}
