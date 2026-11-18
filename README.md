# ECS Scene Renderer

A lightweight, Bevy-inspired ECS (Entity Component System) scene manager and renderer written in C++17, supporting both 2D and 3D graphics.

## Features

- **ECS Architecture**: Built on [EnTT](https://github.com/skypjack/entt) with a modular plugin system, schedules, resources, and bundles.
- **3D & PBR Rendering**: Model loading via Assimp (`.obj`), lighting, and materials.
- **2D Sprite Rendering**: Efficient 2D sprites with hierarchical transformations.
- **3D Text Rendering**: Dynamic TrueType font mesh generation using `ttf2mesh`.
- **Asset Management**: Handle-based generic asset loading and management (`R_Assets`).
- **Scene Hierarchy**: Parent-child spatial transform propagation.
- **Input & Windowing**: GLFW-based window creation and event-driven input server.

## Dependencies

- **EnTT**: Fast and reliable ECS framework
- **Assimp**: 3D model importer
- **GLFW**: Windowing and OpenGL context
- **pathfinder-cpp**: Vector graphics rendering backend
- **GSL & stduuid**: Utility libraries

## Build & Run Examples

Requires CMake (3.16.1+) and a C++17 compatible compiler.

```bash
mkdir build
cmake -B build
cmake --build build
```

Compiled examples (`pbr`, `sprite`, `text_3d`, `render_to_texture`) will be located in the `bin/` directory.

## Project Structure

```text
src/
├── app/       # Application & Plugin framework
├── ecs/       # ECS core (World, Schedule, Entity)
├── render/    # Render pipeline, graph, and API
├── pbr/       # PBR and 3D mesh rendering
├── sprite/    # 2D sprite rendering
├── text_3d/   # 3D text and font processing
├── asset/     # Asset handles and management
├── input/     # Input event server
├── geometry/  # Math library (Vec, Mat, Quat)
└── transform/ # Transform & scene hierarchy
```
