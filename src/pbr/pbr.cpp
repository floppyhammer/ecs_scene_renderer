#include "pbr.h"

#include "../render/render_resource/pipeline_specializer.h"
#include "light.h"
#include "material.h"
#include "mesh.h"

namespace esr {

void PbrPlugin::build(App& app) {
    app.add_plugin(std::make_shared<MaterialPlugin>());
    app.add_plugin(std::make_shared<MeshRenderPlugin>());

    auto res = app.get_sub_app("Render");
    if (!res.is_ok()) {
        return;
    }

    const auto& render_app = res.unwrap();

    render_app->insert_resource(std::make_shared<R_SpecializedRenderPipelines<R_PbrPipeline>>());
    render_app->insert_resource(std::make_shared<R_SpecializedDescriptorSets<R_PbrPipeline, PbrDescriptorSetKey>>());

    // Lights.
    render_app->insert_resource(std::make_shared<R_ExtractedLights>());
    render_app->insert_resource(std::make_shared<R_LightUniform>());
    render_app->add_system(std::make_shared<ScheduleLabelExtract>(), extract_lights);
    render_app->add_system(std::make_shared<ScheduleLabelPrepare>(), prepare_light_uniform);
}

Entity MeshBundle::spawn(World& world) {
    auto entity = world.spawn();

    world.add_component(entity, mesh);
    world.add_component(entity, material);
    world.add_component(entity, transform);
    world.add_component(entity, global_transform);
    world.add_component(entity, visibility);

    return entity;
}

} // namespace esr
