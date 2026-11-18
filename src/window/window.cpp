#include "window.h"

#include "../app/app.h"
#include "../geometry/vec2.h"

namespace esr {

const Vec2I DEFAULT_WINDOW_SIZE(1280, 720);

void WindowPlugin::build(App& app) {
    // Add a primary window.
    auto entity = app.world->spawn();
    app.world->add_component(entity, C_Window{"Primary Window", WindowResolution{DEFAULT_WINDOW_SIZE, 1.0, {}}});
    app.world->add_component(entity, C_PrimaryWindow());
}

} // namespace esr
