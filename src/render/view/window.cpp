#include "window.h"

#include <GLFW/glfw3.h>

#include <iostream>

#include "../../app/app.h"
#include "../../input/input.h"
#include "../../window/window.h"
#include "../render.h"
#include "../render_schedule.h"

namespace esr {

#ifndef ESR_NO_WINDOW

void extract_windows(World& world) {
    auto main_world = world.resource<std::shared_ptr<R_MainWorld>>()->world_;

    auto& extracted_windows = world.resource_mut<std::shared_ptr<R_ExtractedWindows>>();

    // Find the primary window.
    auto primary_query = main_world->registry.view<C_Window, C_PrimaryWindow>();
    for (auto [entity, window, primary_window] : primary_query.each()) {
        extracted_windows->primary = entity;
    }

    auto query = main_world->registry.view<C_Window>();

    for (auto [entity, window] : query.each()) {
        auto new_size = window.resolution.physical_size;

        auto iter = extracted_windows->windows.find(entity);

        // Add newly extracted window.
        if (iter == extracted_windows->windows.end()) {
            extracted_windows->windows[entity] = {
                entity,
                new_size,
                {},
                {},
                {},
                {},
                {},
                false,
            };
        }

        auto& extracted_window = extracted_windows->windows[entity];

        extracted_window.size_changed = new_size != extracted_window.physical_size;

        if (extracted_window.size_changed) {
            std::cout << "Window size changed from " << extracted_window.physical_size << " to " << new_size
                      << std::endl;
            extracted_window.physical_size = new_size;
        }
    }
}

/// Creates and (re)configures window surfaces, and obtains a swapchain texture for rendering.
void prepare_windows(World& world) {
    auto& extracted_windows = world.resource_mut<std::shared_ptr<R_ExtractedWindows>>();
    auto& windows = world.resource_mut<std::shared_ptr<R_Windows>>();
    auto& surfaces = world.resource_mut<std::shared_ptr<R_WindowSurfaces>>();

    auto main_world = world.resource<std::shared_ptr<R_MainWorld>>()->world_;
    auto input_server = main_world->resource_mut<std::shared_ptr<R_InputServer>>();

    for (auto& [entity, window] : extracted_windows->windows) {
        auto w = windows->get_window_by_entity(entity);

        // Extracted window has not been prepared.
        if (!w.has_value()) {
    // TODO: handle multiple window scenario.
    // Create platform window builder.
    #ifdef __APPLE__
            auto backend = Pathfinder::BackendType::Metal;
    #else
            auto backend = Pathfinder::BackendType::Opengl;
        #ifdef ESR_USE_VULKAN
            backend = Pathfinder::BackendType::Vulkan;
        #endif
    #endif

            auto window_builder =
                Pathfinder::WindowBuilder::new_impl(backend, {window.physical_size.x, window.physical_size.y});

            auto driver = window_builder->request_device();
            auto queue = window_builder->create_queue();

            // Insert render context resources.
            world.insert_resource(std::make_shared<R_RenderDevice>(driver, window_builder));
            world.insert_resource(std::make_shared<R_RenderQueue>(queue, driver));

            auto pf_window = window_builder->get_window(0).lock();
            auto glfw_handle = static_cast<GLFWwindow*>(pf_window->get_glfw_handle());

            auto new_glfw_window = std::make_shared<WrappedWindow>(glfw_handle);
            const WindowId window_id = {new_glfw_window->get_handle()};

            windows->windows[window_id] = new_glfw_window;
            windows->entity_to_window_id[entity] = window_id;
            windows->window_id_to_entity[window_id] = entity;

            input_server->initialize_window_callbacks(new_glfw_window->get_handle());

            std::cout << "Created a new window." << std::endl;
        }

        auto surface_data = surfaces->get(entity);
        if (!surface_data.has_value()) {
            auto& device = world.resource_mut<std::shared_ptr<R_RenderDevice>>();
            auto pf_window = device->window_builder_->get_window(0).lock();
            auto swapchain = pf_window->get_swap_chain(device->driver_, Pathfinder::PresentMode::Fifo);

            auto format = swapchain->get_surface_format();

            window.swapchain_ = swapchain;

            surface_data = SurfaceData{swapchain, format};

            surfaces->surfaces[entity] = surface_data.value();

            std::cout << "Created a new swap chain." << std::endl;
        } else {
            window.swapchain_ = surface_data->swap_chain;
        }
    }
}

void flush_system(World& world) {
    auto& surfaces = world.resource_mut<std::shared_ptr<R_WindowSurfaces>>();
    auto& windows = world.resource_mut<std::shared_ptr<R_Windows>>();

    for (auto& [entity, window_id] : windows->entity_to_window_id) {
        auto window = windows->get_window_by_entity(entity);
        auto surface = surfaces->surfaces[entity];

        if (window.has_value()) {
            surface.swap_chain->present();
        }
    }

    glfwPollEvents();

    for (auto& [entity, window_id] : windows->entity_to_window_id) {
        auto window = windows->get_window_by_entity(entity);
        if (window.has_value() && glfwWindowShouldClose(window.value()->get_handle())) {
            windows->should_close = true;
        }
    }
}

void WindowRenderPlugin::build(App& app) {
    auto res = app.get_sub_app("Render");
    if (!res.is_ok()) {
        return;
    }

    auto render_app = res.unwrap();

    if (!glfwInit()) {
        abort();
    }

    render_app->insert_resource(std::make_shared<R_Windows>());
    render_app->insert_resource(std::make_shared<R_ExtractedWindows>());
    render_app->insert_resource(std::make_shared<R_WindowSurfaces>());
    render_app->add_system(std::make_shared<ScheduleLabelExtract>(), extract_windows);
    render_app->add_system(std::make_shared<ScheduleLabelPrepare>(), prepare_windows);
}

#endif

} // namespace esr
