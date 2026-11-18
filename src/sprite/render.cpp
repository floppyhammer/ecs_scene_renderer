#include "render.h"

#include "../asset/asset.h"
#include "../core_pipeline/core_2d/core_2d.h"
#include "../ecs/world.h"
#include "../render/render.h"
#include "../render/render_asset.h"
#include "../render/render_phase/render_phase.h"
#include "../render/render_resource/pipeline_specializer.h"
#include "../render/texture/image.h"
#include "../render/view/view.h"
#include "../render/view/window.h"
#include "sprite.h"
#include "pathfinder/gpu/device.h"
#include "pathfinder/gpu/shader_translator.h"

namespace esr {

std::pair<std::shared_ptr<Pathfinder::ShaderModule>, std::shared_ptr<Pathfinder::ShaderModule>>
create_sprite_shader_modules(Pathfinder::Device* driver, PipelineKey key) {
    std::string vert_code = R"(
#version 450

layout (std140, binding = 1) uniform View {
    mat4 view_proj;
    mat4 view;
    mat4 projection;
    vec3 world_position;
    float pad0;
    vec4 viewport;
};

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 in_uv;

layout(location = 0) out vec2 frag_uv;

void main() {
    gl_Position = view_proj * vec4(position, 0.0, 1.0);
    frag_uv = in_uv;
}
)";

    std::string frag_code = R"(
#version 450

layout(location = 0) in vec2 frag_uv;

layout(location = 0) out vec4 out_color;

layout(binding = 2) uniform sampler2D input_texture;

void main() {
    out_color = texture(input_texture, frag_uv);
}
)";

    auto vert_translator = std::make_shared<Pathfinder::ShaderTranslator>(Pathfinder::ShaderStage::Vertex);
    auto frag_translator = std::make_shared<Pathfinder::ShaderTranslator>(Pathfinder::ShaderStage::Fragment);

    vert_translator->compile_from_glsl("main", vert_code, false);
    frag_translator->compile_from_glsl("main", frag_code, false);

    auto vert_shader_module = driver->create_shader_module(vert_translator->get_shader(), "sprite_vs");
    auto frag_shader_module = driver->create_shader_module(frag_translator->get_shader(), "sprite_fs");

    return {vert_shader_module, frag_shader_module};
}

std::shared_ptr<Pathfinder::RenderPipeline> build_sprite_pipeline(
    R_RenderDevice& device,
    std::shared_ptr<Pathfinder::SwapChain> swap_chain,
    std::shared_ptr<Pathfinder::DescriptorSetLayout> descriptor_set_layout) {
    auto driver = device.driver_;

    auto [vert_shader_module, frag_shader_module] = create_sprite_shader_modules(driver.get(), PipelineKey());

    auto attribute_descriptions = SpriteVertex::get_attribute_descriptions();

    auto pipeline = driver->create_render_pipeline(
        vert_shader_module,
        frag_shader_module,
        attribute_descriptions,
        Pathfinder::BlendState::from_over(),
        descriptor_set_layout,
        swap_chain->get_surface_format(),
        "sprite_pipeline");

    return pipeline;
}

std::shared_ptr<Pathfinder::DescriptorSet> build_sprite_descriptor_set(
    R_RenderDevice& device,
    std::shared_ptr<Pathfinder::DescriptorSetLayout> layout,
    GpuImage& gpu_image,
    std::shared_ptr<Pathfinder::Buffer> view_buffer,
    size_t view_offset = 0,
    size_t view_range = 0) {
    auto driver = device.driver_;

    auto descriptor_set = driver->create_descriptor_set(layout);

    std::vector<Pathfinder::Descriptor> descriptors;
    if (view_buffer) {
        if (view_range > 0) {
            descriptors.push_back(Pathfinder::Descriptor::uniform(1, view_buffer, view_offset, view_range));
        } else {
            descriptors.push_back(Pathfinder::Descriptor::uniform(1, view_buffer));
        }
    }
    if (gpu_image.texture) {
        descriptors.push_back(Pathfinder::Descriptor::sampled(2, gpu_image.texture, gpu_image.sampler));
    }
    descriptor_set->add_or_update(descriptors);

    return descriptor_set;
}

void extract_sprites(World& world) {
    auto main_world = world.resource<std::shared_ptr<R_MainWorld>>()->world_;

    auto& extracted_sprites = world.resource_mut<std::shared_ptr<R_ExtractedSprites>>();
    extracted_sprites->sprites.clear();

    auto query = main_world->registry.view<C_Visibility, C_Sprite, C_GlobalTransform, C_AssetHandle<Image>>();

    for (auto [entity, visibility, sprite, transform, handle] : query.each()) {
        if (!visibility.visible) {
            continue;
        }

        extracted_sprites->sprites.push_back(ExtractedSprite{
            entity,
            transform,
            sprite.color,
            sprite.flip_x,
            sprite.flip_y,
            sprite.custom_size,
            sprite.rect,
            handle.as_weak(),
        });
    }
}

void queue_sprites(World& world) {
    auto& render_device = world.resource_mut<std::shared_ptr<R_RenderDevice>>();
    auto& render_queue = world.resource_mut<std::shared_ptr<R_RenderQueue>>();
    auto driver = render_device->driver_;

    auto& gpu_images = world.resource_mut<std::shared_ptr<R_RenderAssets<Image, GpuImage>>>();
    auto& sprite_meta = world.resource_mut<std::shared_ptr<R_SpriteMeta>>();

    auto& pipelines = world.resource_mut<std::shared_ptr<R_SpecializedRenderPipelines<R_SpritePipeline>>>();
    auto& pipeline_cache = world.resource_mut<std::shared_ptr<R_RenderPipelineCache>>();
    auto& descriptor_sets =
        world.resource_mut<std::shared_ptr<R_SpecializedDescriptorSets<R_SpritePipeline, C_SpriteBatch>>>();
    auto& descriptor_set_cache = world.resource_mut<std::shared_ptr<R_DescriptorSetCache>>();

    auto& windows = world.resource<std::shared_ptr<R_ExtractedWindows>>();
    auto& surfaces = world.resource<std::shared_ptr<R_WindowSurfaces>>();
    auto swap_chain = surfaces->surfaces[windows->primary.value()].swap_chain;

    auto& view_uniforms = world.resource_mut<std::shared_ptr<R_ViewUniforms>>();

    sprite_meta->vertices.clear();

    auto& draw_functions = world.resource<std::shared_ptr<R_DrawFunctions<Transparent2d>>>();
    auto draw_sprite_function = draw_functions->get_id<DrawSprite<Transparent2d>>();

    uint32_t index = 0;

    auto& extracted_sprites = world.resource<std::shared_ptr<R_ExtractedSprites>>()->sprites;

    auto query = world.registry.view<C_RenderPhase<Transparent2d>, C_ExtractedView, C_ViewUniformOffset>();

    std::vector<Pathfinder::DescriptorLayout> descriptor_layouts = {
        {1, Pathfinder::ShaderStage::VertexAndFragment, Pathfinder::DescriptorType::UniformBuffer},
        {2, Pathfinder::ShaderStage::Fragment, Pathfinder::DescriptorType::Sampler},
    };
    auto descriptor_set_layout = driver->create_descriptor_set_layout(descriptor_layouts);

    for (auto [view_entity, transparent_phase, extracted_view, uniform_offset] : query.each()) {
        auto view_buffer = view_uniforms->uniforms.get_buffer();

        transparent_phase.items.reserve(extracted_sprites.size());

        C_SpriteBatch current_batch;
        Entity current_batch_entity = entt::null;
        auto current_image_size = Vec2F(0);

        for (auto& sprite : extracted_sprites) {
            auto new_batch = C_SpriteBatch{sprite.image_handle, nullptr};

            if (!(new_batch == current_batch)) {
                auto opt = gpu_images->get_gpu_asset(new_batch.image_handle);

                if (opt.has_value()) {
                    auto& gpu_image = opt.value();
                    new_batch.gpu_image_handle = gpu_image.get();

                    current_batch = new_batch;
                    current_image_size = gpu_image->size.to_f32();

                    current_batch_entity = world.spawn();
                    world.add_component(current_batch_entity, current_batch);
                } else {
                    continue;
                }
            }

            auto uvs = QUAD_UVS;

            if (sprite.flip_x) {
                uvs = {uvs[1], uvs[0], uvs[3], uvs[2]};
            }
            if (sprite.flip_y) {
                uvs = {uvs[3], uvs[2], uvs[1], uvs[0]};
            }

            auto quad_size = current_image_size;

            if (sprite.rect.has_value()) {
                auto rect = sprite.rect.value();
                auto rect_size = rect.size();
                for (auto& uv : uvs) {
                    uv = (rect.origin() + uv * rect_size) / current_image_size;
                }
                quad_size = rect_size;
            }

            if (sprite.custom_size.has_value()) {
                quad_size = sprite.custom_size.value();
            }

            std::array<Vec2F, QUAD_VERTEX_POSITIONS.size()> positions;
            for (size_t i = 0; i < QUAD_VERTEX_POSITIONS.size(); i++) {
                auto& quad_pos = QUAD_VERTEX_POSITIONS[i];
                positions[i] = sprite.transform.transform_point(Vec3F(quad_pos * quad_size, 1)).xy();
            }

            for (auto i : QUAD_INDICES) {
                sprite_meta->vertices.push(SpriteVertex{
                    {positions[i].x, positions[i].y},
                    {uvs[i].x, uvs[i].y},
                });
            }

            std::shared_ptr<Pathfinder::DescriptorSet> descriptor_set;

            auto descriptor_set_id = descriptor_sets->find(current_batch);
            if (!descriptor_set_id.has_value()) {
                descriptor_set =
                    build_sprite_descriptor_set(*render_device, descriptor_set_layout, *current_batch.gpu_image_handle, view_buffer, uniform_offset.offset * view_uniforms->uniforms.get_aligned_size(), view_uniforms->uniforms.get_aligned_size());
                descriptor_set_id = descriptor_sets->specialize(*descriptor_set_cache, descriptor_set, current_batch);
            } else {
                descriptor_set = descriptor_set_cache->get_descriptor_set(descriptor_set_id.value()).value();
            }

            auto pipeline_key = std::bitset<MAX_PIPELINE_KEY>();

            auto pipeline_id = pipelines->find(pipeline_key);
            if (!pipeline_id.has_value()) {
                auto pipeline = build_sprite_pipeline(*render_device, swap_chain, descriptor_set_layout);

                pipeline_id = pipelines->specialize(*pipeline_cache, pipeline, pipeline_key);
            }

            uint32_t item_start = index;
            index += QUAD_INDICES.size();
            uint32_t item_end = index;

            transparent_phase.add(std::make_shared<Transparent2d>(current_batch_entity,
                                                                  pipeline_id.value(),
                                                                  descriptor_set_id.value(),
                                                                  draw_sprite_function,
                                                                  Range(item_start, item_end)));
        }

        sprite_meta->vertices.write_buffer(*render_device, *render_queue);
    }
}

} // namespace esr
