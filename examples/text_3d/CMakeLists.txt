# Include esr headers.
include_directories(../../include)

# Build executable.
add_executable(esr_example_text_3d main.cpp)

# Link libs.
target_link_libraries(esr_example_text_3d PUBLIC ecs_scene_renderer)
