#include "mesh.h"

#include "../asset/asset.h"
#include "../core_pipeline/core_3d/core_3d.h"
#include "../render/render.h"
#include "../render/render_asset.h"
#include "../render/render_phase/render_phase.h"
#include "../render/render_resource/pipeline_specializer.h"
#include "../render/render_schedule.h"
#include "../render/view/view.h"
#include "../render/view/window.h"
#include "light.h"
#include "material.h"
#include "shader_assembler.h"

namespace esr {

std::shared_ptr<Pathfinder::RenderPipeline> build_pbr_pipeline(
    R_RenderDevice& device,
    std::shared_ptr<Pathfinder::SwapChain> swap_chain,
    PipelineKey key,
    std::shared_ptr<Pathfinder::DescriptorSetLayout> descriptor_set_layout) {
    auto driver = device.driver_;

    auto [vert_shader_module, frag_shader_module] = create_mesh_shader_modules(driver.get(), key);

    auto attribute_descriptions = Vertex::get_attribute_descriptions();

    Pathfinder::BlendState blend_state{};
    if ((key & PbrPipelineKeys::ALPHA_MODE_BLEND).to_ulong()) {
        blend_state = Pathfinder::BlendState::from_over();
    }

    auto pipeline = driver->create_render_pipeline(
        vert_shader_module,
        frag_shader_module,
        attribute_descriptions,
        blend_state,
        descriptor_set_layout,
        swap_chain->get_surface_format(),
        "pbr_pipeline");

    return pipeline;
}

void update_pbr_descriptor_set(std::shared_ptr<Pathfinder::DescriptorSet> descriptor_set,
                               StandardMaterial& material,
                               std::shared_ptr<Pathfinder::Buffer> view_buffer,
                               size_t view_offset,
                               size_t view_range,
                               std::shared_ptr<Pathfinder::Buffer> mesh_buffer,
                               std::shared_ptr<Pathfinder::Buffer> material_buffer,
                               std::shared_ptr<Pathfinder::Buffer> light_buffer,
                               std::shared_ptr<R_RenderAssets<Image, GpuImage>> image_assets) {
    std::vector<Pathfinder::Descriptor> descriptors;
    if (view_buffer) descriptors.push_back(Pathfinder::Descriptor::uniform(1, view_buffer, view_offset, view_range));
    if (mesh_buffer) descriptors.push_back(Pathfinder::Descriptor::uniform(2, mesh_buffer));
    if (material_buffer) descriptors.push_back(Pathfinder::Descriptor::uniform(3, material_buffer));
    if (light_buffer) descriptors.push_back(Pathfinder::Descriptor::uniform(4, light_buffer));

    if (material.get_base_color_texture().has_value()) {
        auto base_color_image = image_assets->get_gpu_asset(material.get_base_color_texture().value().as_weak());

        if (base_color_image.has_value()) {
            descriptors.push_back(Pathfinder::Descriptor::sampled(5, base_color_image.value()->texture, base_color_image.value()->sampler));
        }
    }

    descriptor_set->add_or_update(descriptors);
}

void extract_meshes(World& world) {
    auto main_world = world.resource<std::shared_ptr<R_MainWorld>>()->world_;

    auto& extracted_meshes = world.resource_mut<std::shared_ptr<R_ExtractedMeshes>>();
    extracted_meshes->meshes.clear();

    auto query = main_world->registry.view<C_Visibility, C_GlobalTransform, C_AssetHandle<Mesh<Vertex>>>();

    for (const auto& [entity, visibility, transform, mesh_handle] : query.each()) {
        if (!visibility.visible) {
            continue;
        }

        auto transform_matrix = transform.compute_matrix();

        bool has_material = main_world->registry.any_of<C_AssetHandle<StandardMaterial>>(entity);

        std::optional<C_AssetHandleWeak<StandardMaterial>> material_weak_handle;
        if (has_material) {
            auto material_handle = main_world->registry.get<C_AssetHandle<StandardMaterial>>(entity);
            material_weak_handle = material_handle.as_weak();
        }

        extracted_meshes->meshes.push_back({mesh_handle.as_weak(), material_weak_handle, transform_matrix});
    }
}

void prepare_mesh_uniforms(World& world) {
    auto& render_device = world.resource_mut<std::shared_ptr<R_RenderDevice>>();
    auto& render_queue = world.resource_mut<std::shared_ptr<R_RenderQueue>>();

    auto& mesh_uniform_storage = world.resource_mut<std::shared_ptr<R_MeshUniformStorage>>();
    auto& extracted_meshes = world.resource_mut<std::shared_ptr<R_ExtractedMeshes>>();

    auto mesh_count = extracted_meshes->meshes.size();

    for (size_t i = 0; i < mesh_count; i++) {
        auto& mesh = extracted_meshes->meshes[i];

        std::shared_ptr<UniformBuffer<MeshUniforms>> buffer;

        if (i >= mesh_uniform_storage->buffers.size()) {
            buffer = std::make_shared<UniformBuffer<MeshUniforms>>();
            mesh_uniform_storage->buffers.push_back(buffer);
        } else {
            buffer = mesh_uniform_storage->buffers[i];
        }

        buffer->set_value(MeshUniforms{mesh.transform});

        buffer->write_buffer(*render_device, *render_queue);
    }
}

void queue_meshes(World& world) {
    auto& render_device = world.resource_mut<std::shared_ptr<R_RenderDevice>>();

    auto driver = render_device->driver_;

    auto& gpu_images = world.resource_mut<std::shared_ptr<R_RenderAssets<Image, GpuImage>>>();
    auto& gpu_meshes = world.resource_mut<std::shared_ptr<R_RenderAssets<Mesh<Vertex>, GpuMesh<Vertex>>>>();

    auto& extracted_materials = world.resource_mut<std::shared_ptr<R_ExtractedMaterials>>();

    auto& pipelines = world.resource_mut<std::shared_ptr<R_SpecializedRenderPipelines<R_PbrPipeline>>>();
    auto& pipeline_cache = world.resource_mut<std::shared_ptr<R_RenderPipelineCache>>();

    auto& descriptor_sets =
        world.resource_mut<std::shared_ptr<R_SpecializedDescriptorSets<R_PbrPipeline, PbrDescriptorSetKey>>>();
    auto& descriptor_set_cache = world.resource_mut<std::shared_ptr<R_DescriptorSetCache>>();

    auto& view_uniforms = world.resource_mut<std::shared_ptr<R_ViewUniforms>>();
    auto& mesh_uniform_storage = world.resource_mut<std::shared_ptr<R_MeshUniformStorage>>();
    auto& material_uniform_storage = world.resource_mut<std::shared_ptr<R_MaterialUniformStorage>>();
    auto& light_uniform = world.resource_mut<std::shared_ptr<R_LightUniform>>();

    auto& windows = world.resource<std::shared_ptr<R_ExtractedWindows>>();
    auto& surfaces = world.resource<std::shared_ptr<R_WindowSurfaces>>();
    auto swap_chain = surfaces->surfaces[windows->primary.value()].swap_chain;

    auto& draw_functions_opaque = world.resource<std::shared_ptr<R_DrawFunctions<Opaque3d>>>();
    auto draw_mesh_function_opaque = draw_functions_opaque->get_id<DrawMesh<Opaque3d>>();

    auto& draw_functions_transparent = world.resource<std::shared_ptr<R_DrawFunctions<Transparent3d>>>();
    auto draw_mesh_function_transparent = draw_functions_transparent->get_id<DrawMesh<Transparent3d>>();

    auto& extracted_meshes = world.resource_mut<std::shared_ptr<R_ExtractedMeshes>>();
    auto mesh_count = extracted_meshes->meshes.size();

    std::vector<Pathfinder::DescriptorLayout> descriptor_layouts = {
        {1, Pathfinder::ShaderStage::VertexAndFragment, Pathfinder::DescriptorType::UniformBuffer},
        {2, Pathfinder::ShaderStage::Vertex, Pathfinder::DescriptorType::UniformBuffer},
        {3, Pathfinder::ShaderStage::Fragment, Pathfinder::DescriptorType::UniformBuffer},
        {4, Pathfinder::ShaderStage::Fragment, Pathfinder::DescriptorType::UniformBuffer},
        {5, Pathfinder::ShaderStage::Fragment, Pathfinder::DescriptorType::Sampler},
    };
    auto descriptor_set_layout = driver->create_descriptor_set_layout(descriptor_layouts);

    auto query =
        world.registry
            .view<C_RenderPhase<Opaque3d>, C_RenderPhase<Transparent3d>, C_ExtractedView, C_ViewUniformOffset>();

    for (auto [view_entity, opaque_phase, transparent_phase, extracted_view, uniform_offset] : query.each()) {
        opaque_phase.items.reserve(mesh_count);
        transparent_phase.items.reserve(mesh_count);

        for (size_t i = 0; i < mesh_count; i++) {
            auto& mesh = extracted_meshes->meshes[i];

            auto gpu_mesh_optional = gpu_meshes->get_gpu_asset(mesh.handle);

            if (!gpu_mesh_optional.has_value()) {
                continue;
            }

            auto gpu_mesh = gpu_mesh_optional.value().get();

            std::shared_ptr<UniformBuffer<MaterialUniform>> material_uniform_buffer;

            StandardMaterial material;

            if (mesh.material.has_value()) {
                material = *(mesh.material.value().get_data());

                auto material_handle = mesh.material.value();

                auto iter = std::find(
                    extracted_materials->extracted.begin(), extracted_materials->extracted.end(), material_handle);

                if (iter == extracted_materials->extracted.end()) {
                    abort();
                } else {
                    int material_index_in_extracted = iter - extracted_materials->extracted.begin();
                    material_uniform_buffer = material_uniform_storage->buffers[material_index_in_extracted];
                }
            } else {
                material_uniform_buffer = material_uniform_storage->default_buffer;
            }

            PbrDescriptorSetKey descriptor_set_key = {material,
                                                      uniform_offset.offset,
                                                      mesh_uniform_storage->buffers[i],
                                                      material_uniform_buffer,
                                                      light_uniform->uniform};

            std::shared_ptr<Pathfinder::DescriptorSet> descriptor_set;

            auto descriptor_set_id = descriptor_sets->find(descriptor_set_key);

            if (!descriptor_set_id.has_value()) {
                descriptor_set = driver->create_descriptor_set(descriptor_set_layout);

                descriptor_set_id =
                    descriptor_sets->specialize(*descriptor_set_cache, descriptor_set, descriptor_set_key);
            } else {
                descriptor_set = descriptor_set_cache->get_descriptor_set(descriptor_set_id.value()).value();
            }

            update_pbr_descriptor_set(descriptor_set,
                                      material,
                                      view_uniforms->uniforms.get_buffer(),
                                      uniform_offset.offset * view_uniforms->uniforms.get_aligned_size(),
                                      view_uniforms->uniforms.get_aligned_size(),
                                      mesh_uniform_storage->buffers[i]->get_buffer(),
                                      material_uniform_buffer->get_buffer(),
                                      light_uniform->uniform->get_buffer(),
                                      gpu_images);

            auto pipeline_key = material.get_pipeline_layout().get_pipeline_key();

            auto pipeline_id = pipelines->find(pipeline_key);

            if (!pipeline_id.has_value()) {
                auto pipeline = build_pbr_pipeline(*render_device, swap_chain, pipeline_key, descriptor_set_layout);

                pipeline_id = pipelines->specialize(*pipeline_cache, pipeline, pipeline_key);
            }

            auto mesh_entity = world.spawn();
            world.add_component(mesh_entity, C_RenderableMesh{gpu_mesh, material});

            if (material.get_alpha_mode() != AlphaMode::Blend) {
                opaque_phase.add(std::make_shared<Opaque3d>(
                    mesh_entity, pipeline_id.value(), descriptor_set_id.value(), draw_mesh_function_opaque, 0));
            } else {
                transparent_phase.add(std::make_shared<Transparent3d>(
                    mesh_entity, pipeline_id.value(), descriptor_set_id.value(), draw_mesh_function_transparent, 0));
            }
        }
    }
}

void MeshRenderPlugin::build(App& app) {
    auto res = app.get_sub_app("Render");
    if (!res.is_ok()) {
        return;
    }

    const auto& render_app = res.unwrap();

    render_app->insert_resource(std::make_shared<R_ExtractedMeshes>());
    render_app->insert_resource(std::make_shared<R_MeshUniformStorage>());

    auto& draw_functions_opaque = render_app->world->resource_mut<std::shared_ptr<R_DrawFunctions<Opaque3d>>>();
    draw_functions_opaque->add<DrawMesh<Opaque3d>>(std::make_shared<DrawMesh<Opaque3d>>());

    auto& draw_functions_transparent =
        render_app->world->resource_mut<std::shared_ptr<R_DrawFunctions<Transparent3d>>>();
    draw_functions_transparent->add<DrawMesh<Transparent3d>>(std::make_shared<DrawMesh<Transparent3d>>());

    render_app->add_system(std::make_shared<ScheduleLabelExtract>(), extract_meshes);
    render_app->add_system(std::make_shared<ScheduleLabelPrepare>(), prepare_mesh_uniforms);
    render_app->add_system(std::make_shared<ScheduleLabelQueue>(), queue_meshes);
}

} // namespace esr
