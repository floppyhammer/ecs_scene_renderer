#include "input.h"

#include <GLFW/glfw3.h>

namespace esr {

void R_InputServer::initialize_window_callbacks(GLFWwindow *window) {
    // A lambda function that doesn't capture anything can be implicitly converted to a regular function pointer.
    auto cursor_position_callback = [](GLFWwindow *window, double x_pos, double y_pos) {
        InputEvent input_event{};
        input_event.type = InputEventType::MouseMotion;
        input_event.window = window;
        input_event.args.mouse_motion.position = {(float)x_pos, (float)y_pos};

        input_server_ptr->last_cursor_position = input_server_ptr->cursor_position;
        input_server_ptr->cursor_position = {(float)x_pos, (float)y_pos};
        input_event.args.mouse_motion.relative =
            input_server_ptr->cursor_position - input_server_ptr->last_cursor_position;
        input_server_ptr->events.push_back(input_event);
    };
    glfwSetCursorPosCallback(window, cursor_position_callback);

    auto cursor_button_callback = [](GLFWwindow *window, int button, int action, int mods) {
        InputEvent input_event{};
        input_event.type = InputEventType::MouseButton;
        input_event.window = window;
        input_event.args.mouse_button.button = button;
        input_event.args.mouse_button.pressed = action == GLFW_PRESS;

        input_event.args.mouse_button.position = input_server_ptr->cursor_position;
        input_server_ptr->events.push_back(input_event);
    };
    glfwSetMouseButtonCallback(window, cursor_button_callback);

    auto cursor_scroll_callback = [](GLFWwindow *window, double x_offset, double y_offset) {
        InputEvent input_event{};
        input_event.type = InputEventType::MouseScroll;
        input_event.window = window;
        input_event.args.mouse_scroll.x_delta = x_offset;
        input_event.args.mouse_scroll.y_delta = y_offset;

        input_server_ptr->events.push_back(input_event);
    };
    glfwSetScrollCallback(window, cursor_scroll_callback);

    auto key_callback = [](GLFWwindow *window, int key, int scancode, int action, int mods) {
        InputEvent input_event{};
        input_event.type = InputEventType::Key;
        input_event.window = window;
        input_event.args.key.pressed = action == GLFW_PRESS;
        input_event.args.key.repeated = action == GLFW_REPEAT;

        switch (key) {
            case GLFW_KEY_BACKSPACE: {
                input_event.args.key.key = KeyCode::Backspace;
            } break;
            case GLFW_KEY_LEFT: {
                input_event.args.key.key = KeyCode::Left;
            } break;
            case GLFW_KEY_RIGHT: {
                input_event.args.key.key = KeyCode::Right;
            } break;
            case GLFW_KEY_UP: {
                input_event.args.key.key = KeyCode::Up;
            } break;
            case GLFW_KEY_DOWN: {
                input_event.args.key.key = KeyCode::Down;
            } break;
            default: {
                input_event.args.key.key = KeyCode::Unknown;
            }
        }

        input_server_ptr->events.push_back(input_event);
    };
    glfwSetKeyCallback(window, key_callback);

    auto character_callback = [](GLFWwindow *window, unsigned int codepoint) {
        InputEvent input_event{};
        input_event.type = InputEventType::Text;
        input_event.window = window;
        input_event.args.text.codepoint = codepoint;

        input_server_ptr->events.push_back(input_event);
    };
    glfwSetCharCallback(window, character_callback);
}

void R_InputServer::set_cursor_captured(GLFWwindow *window, bool captured) {
    glfwSetInputMode(window, GLFW_CURSOR, captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

void R_InputServer::hide_cursor(GLFWwindow *window) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}

const std::vector<InputEvent> &R_InputServer::get_events() const {
    return events;
}

void R_InputServer::clear_events() {
    events.clear();
}

void clear_input_events(World &world) {
    auto input_server = world.resource_mut<std::shared_ptr<R_InputServer>>();
    input_server->clear_events();
}

void esr::InputPlugin::build(esr::App &app) {
    app.insert_resource(std::make_shared<R_InputServer>());
    app.add_system(std::make_shared<ScheduleLabelLast>(), clear_input_events);

    input_server_ptr = app.world->resource_mut<std::shared_ptr<R_InputServer>>().get();
}

} // namespace esr
