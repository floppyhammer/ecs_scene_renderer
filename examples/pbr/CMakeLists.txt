# Include esr headers.
include_directories(../../include)

# Build executable.
add_executable(esr_example_pbr main.cpp)

# Link libs.
target_link_libraries(esr_example_pbr PUBLIC ecs_scene_renderer)
