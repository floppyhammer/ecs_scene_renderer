#include "material.h"

#include "../asset/asset.h"
#include "../ecs/world.h"
#include "../render/mesh/vertex.h"
#include "../render/render.h"
#include "../render/render_asset.h"

namespace esr {

const PipelineKey PbrPipelineKeys::BASE_COLOR_TEXTURE = {0b1};
const PipelineKey PbrPipelineKeys::NORMAL_TEXTURE = {0b10};
const PipelineKey PbrPipelineKeys::ALPHA_MODE_OPAQUE = {0b100};
const PipelineKey PbrPipelineKeys::ALPHA_MODE_BLEND = {0b1000};
const PipelineKey PbrPipelineKeys::ALPHA_MODE_MASK = {0b10000};
const PipelineKey PbrPipelineKeys::UNLIT = {0b100000};

void StandardMaterial::set_base_color_texture(C_AssetHandle<Image> new_texture) {
    base_color_texture = new_texture;
}

std::optional<C_AssetHandle<Image>> StandardMaterial::get_base_color_texture() const {
    return base_color_texture;
}

std::shared_ptr<Pathfinder::RenderPipeline> StandardMaterial::generate_pipeline() {
    return std::shared_ptr<Pathfinder::RenderPipeline>();
}

std::shared_ptr<Pathfinder::DescriptorSet> StandardMaterial::generate_descriptor_set() {
    return std::shared_ptr<Pathfinder::DescriptorSet>();
}

StandardMaterialPipelineLayout StandardMaterial::get_pipeline_layout() const {
    return StandardMaterialPipelineLayout{base_color_texture.has_value(), normal_map_texture.has_value(), alpha_mode_, unlit_};
}

void extract_materials(World& world) {
    auto main_world = world.resource<std::shared_ptr<R_MainWorld>>()->world_;

    auto& extracted_materials = world.resource_mut<std::shared_ptr<R_ExtractedMaterials>>();
    extracted_materials->extracted.clear();

    auto query = main_world->registry.view<C_Visibility, C_GlobalTransform, C_AssetHandle<StandardMaterial>>();

    for (auto [entity, visibility, transform, material_handle] : query.each()) {
        if (!visibility.visible) {
            continue;
        }

        extracted_materials->extracted.push_back(material_handle.as_weak());
    }
}

void prepare_material_uniforms(World& world) {
    auto& render_device = world.resource_mut<std::shared_ptr<R_RenderDevice>>();
    auto& render_queue = world.resource_mut<std::shared_ptr<R_RenderQueue>>();

    auto driver = render_device->driver_;

    auto& material_uniform_storage = world.resource_mut<std::shared_ptr<R_MaterialUniformStorage>>();
    auto& extracted_materials = world.resource_mut<std::shared_ptr<R_ExtractedMaterials>>();

    auto material_count = extracted_materials->extracted.size();

    if (!material_uniform_storage->default_buffer) {
        auto buffer = std::make_shared<UniformBuffer<MaterialUniform>>();

        MaterialUniform uniform;
        uniform.base_color[0] = 1.0;
        uniform.base_color[1] = 1.0;
        uniform.base_color[2] = 1.0;
        uniform.base_color[3] = 1.0;
        uniform.alpha_cutoff = 0.0;

        buffer->set_value(uniform);

        material_uniform_storage->default_buffer = buffer;
    }

    // Mesh uniform is the same for a mesh in different views.
    for (int i = 0; i < material_count; i++) {
        auto& material = extracted_materials->extracted[i];

        if (i >= material_uniform_storage->buffers.size()) {
            auto new_buffer = std::make_shared<UniformBuffer<MaterialUniform>>();
            material_uniform_storage->buffers.push_back(new_buffer);
        }

        auto color_vec = material.get_data()->get_base_color().to_vec();

        auto& buffer = material_uniform_storage->buffers[i];

        MaterialUniform uniform;
        uniform.base_color[0] = color_vec.x;
        uniform.base_color[1] = color_vec.y;
        uniform.base_color[2] = color_vec.z;
        uniform.base_color[3] = color_vec.w;
        uniform.metallic = material.get_data()->get_metallic();
        uniform.roughness = material.get_data()->get_roughness();
        uniform.ao = material.get_data()->get_ao();
        uniform.alpha_cutoff =
            material.get_data()->get_alpha_cutoff().has_value() ? material.get_data()->get_alpha_cutoff().value() : 0.0;

        buffer->set_value(uniform);

        buffer->write_buffer(*render_device, *render_queue);
    }
}

void MaterialPlugin::build(App& app) {
    auto res = app.get_sub_app("Render");
    if (!res.is_ok()) {
        return;
    }

    app.add_plugin(std::make_shared<AssetPlugin<StandardMaterial>>());

    const auto& render_app = res.unwrap();

    render_app->insert_resource(std::make_shared<R_ExtractedMaterials>());
    render_app->insert_resource(std::make_shared<R_PbrPipeline>());
    render_app->insert_resource(std::make_shared<R_MaterialUniformStorage>());

    render_app->add_system(std::make_shared<ScheduleLabelPrepare>(), prepare_material_uniforms);
    render_app->add_system(std::make_shared<ScheduleLabelExtract>(), extract_materials);
}

} // namespace esr
