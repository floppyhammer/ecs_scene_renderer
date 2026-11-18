#ifndef ESR_INPUT_H
#define ESR_INPUT_H

#include "../app/app.h"
#include "../app/plugin.h"
#include "../asset/asset.h"
#include "../ecs/bundle.h"
#include "../ecs/world.h"
#include "../geometry/vec2.h"

namespace esr {

enum class InputEventType {
    MouseButton = 0,
    MouseMotion,
    MouseScroll,
    Key,
    Text,
    Max,
};

enum class KeyCode {
    Unknown,
    Backspace,
    Left,
    Right,
    Up,
    Down,
};

class InputEvent {
public:
    InputEventType type = InputEventType::Max;
    GLFWwindow *window{};

    union Args {
        struct {
            KeyCode key;
            bool pressed;
            bool repeated;
        } key{};
        struct {
            uint8_t button;
            bool pressed;
            Vec2F position;
        } mouse_button;
        struct {
            float x_delta;
            float y_delta;
        } mouse_scroll;
        struct {
            Vec2F relative;
            Vec2F position;
        } mouse_motion;
        struct {
            uint32_t codepoint;
        } text;
    } args{};
};

class R_InputServer {
public:
    void initialize_window_callbacks(GLFWwindow *window);

    void set_cursor_captured(GLFWwindow *window, bool captured);

    void hide_cursor(GLFWwindow *window);

    const std::vector<InputEvent> &get_events() const;

    void clear_events();

private:
    Vec2F cursor_position;
    Vec2F last_cursor_position;

    std::vector<InputEvent> events;
};

static R_InputServer *input_server_ptr;

class InputPlugin : public Plugin {
    PLUGIN_INIT

    void build(App &app) override;
};

} // namespace esr

#endif // ESR_INPUT_H
