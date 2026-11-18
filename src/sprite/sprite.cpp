#include "sprite.h"

#include "../app/app.h"
#include "../core_pipeline/core_2d/core_2d.h"
#include "../render/render_resource/pipeline_specializer.h"
#include "../render/render_schedule.h"
#include "render.h"

namespace esr {

void SpritePlugin::build(App& app) {
    auto res = app.get_sub_app("Render");
    if (!res.is_ok()) {
        return;
    }

    auto& render_app = res.unwrap();

    render_app->insert_resource(std::make_shared<R_SpritePipeline>());
    render_app->insert_resource(std::make_shared<R_SpriteMeta>());
    render_app->insert_resource(std::make_shared<R_ExtractedSprites>());
    render_app->insert_resource(std::make_shared<R_SpecializedRenderPipelines<R_SpritePipeline>>());
    render_app->insert_resource(std::make_shared<R_SpecializedDescriptorSets<R_SpritePipeline, C_SpriteBatch>>());

    auto& draw_functions = render_app->world->resource_mut<std::shared_ptr<R_DrawFunctions<Transparent2d>>>();
    draw_functions->add<DrawSprite<Transparent2d>>(std::make_shared<DrawSprite<Transparent2d>>());

    render_app->add_system(std::make_shared<ScheduleLabelExtract>(), extract_sprites);
    render_app->add_system(std::make_shared<ScheduleLabelQueue>(), queue_sprites);
}

} // namespace esr
