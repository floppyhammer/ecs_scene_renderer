cmake_minimum_required(VERSION 3.16.1)

project(ecs_scene_renderer)

if (UNIX AND NOT APPLE AND NOT ANDROID)
    set(LINUX ON)
endif ()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

# Compiler specific flags.
if (MSVC)
    # Fix "too many characters in constant" error with MSVC.
    string(APPEND CMAKE_CXX_FLAGS " /utf-8")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-sign-compare -Wno-nontrivial-memcall")
endif ()

option(ESR_NO_WINDOW "Offscreen rendering support" OFF)

if (ESR_NO_WINDOW)
    add_compile_definitions(ESR_NO_WINDOW)
endif ()

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
        src/*.cpp
        third_party/ttf2mesh/ttf2mesh.c
        third_party/xatlas/xatlas.cpp)

# Include external and third-party headers.
include_directories(external third_party)

include_directories(external/entt/src)
add_subdirectory(external/entt)

include_directories(external/GSL/include)
add_subdirectory(external/GSL)

include_directories(external/stduuid/include)
add_subdirectory(external/stduuid)

include_directories(external/assimp/include)
# Where the generated config file located.
include_directories(${CMAKE_CURRENT_BINARY_DIR}/external/assimp/include)
include_directories(external/assimp/build/include)
add_subdirectory(external/assimp)

include_directories(external/glfw/include)

# Avoid link errors like "cannot open file lib\SPIRVd.lib".
set(BUILD_SHARED_LIBS OFF)

include_directories(external/pathfinder-cpp)
set(PATHFINDER_BUILD_DEMO OFF)
set(PATHFINDER_RUNTIME_SHADER_COMPLICATION ON)
add_subdirectory(external/pathfinder-cpp)

add_library(ecs_scene_renderer STATIC ${SOURCE_FILES})
target_link_libraries(ecs_scene_renderer PUBLIC pathfinder glfw assimp stduuid)

if (LINUX)
    target_link_libraries(ecs_scene_renderer PUBLIC uuid)
endif ()

# Add examples.
add_subdirectory(examples/sprite)
add_subdirectory(examples/pbr)
add_subdirectory(examples/render_to_texture)
add_subdirectory(examples/text_3d)
