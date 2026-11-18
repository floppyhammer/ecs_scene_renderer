# Include esr headers.
include_directories(../../include)

# Build executable.
add_executable(esr_example_render_to_texture main.cpp)

# Link libs.
target_link_libraries(esr_example_render_to_texture PUBLIC ecs_scene_renderer)
