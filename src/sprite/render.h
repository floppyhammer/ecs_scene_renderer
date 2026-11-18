#ifndef ESR_SPRITE_RENDER_H
#define ESR_SPRITE_RENDER_H

#include "../asset/asset_handle.h"
#include "../ecs/world.h"
#include "../geometry/vec2.h"
#include "../render/color.h"
#include "../render/render.h"
#include "../render/render_api.h"
#include "../render/render_phase/draw.h"
#include "../render/render_resource/pipeline_cache.h"
#include "../render/render_resource/render_resource.h"
#include "../render/texture/image.h"

namespace esr {

class World;

const std::array<uint32_t, 6> QUAD_INDICES = {0, 2, 3, 0, 1, 2};

const std::array<Vec2F, 4> QUAD_VERTEX_POSITIONS = {
    Vec2F(-0.5, 0.5),
    Vec2F(0.5, 0.5),
    Vec2F(0.5, -0.5),
    Vec2F(-0.5, -0.5),
};

const std::array<Vec2F, 4> QUAD_UVS = {
    Vec2F(0., 1.),
    Vec2F(1., 1.),
    Vec2F(1., 0.),
    Vec2F(0., 0.),
};

struct ExtractedSprite {
    Entity entity = entt::null;
    C_GlobalTransform transform;
    Color color;

    bool flip_x = false;
    bool flip_y = false;
    std::optional<Vec2F> custom_size;
    std::optional<RectF> rect;
    C_AssetHandleWeak<Image> image_handle;
};

struct R_ExtractedSprites {
    std::vector<ExtractedSprite> sprites;
};

struct SpriteVertex {
    float position[2];
    float uv[2];

    static std::vector<Pathfinder::VertexInputAttributeDescription> get_attribute_descriptions() {
        std::vector<Pathfinder::VertexInputAttributeDescription> attributes;
        attributes.reserve(2);
        attributes.push_back({0, 2, Pathfinder::DataType::f32, (uint32_t)sizeof(SpriteVertex), offsetof(SpriteVertex, position), Pathfinder::VertexInputRate::Vertex});
        attributes.push_back({0, 2, Pathfinder::DataType::f32, (uint32_t)sizeof(SpriteVertex), offsetof(SpriteVertex, uv), Pathfinder::VertexInputRate::Vertex});
        return attributes;
    }
};

struct R_SpriteMeta {
    BufferVec<SpriteVertex> vertices;

    R_SpriteMeta() {
        vertices = BufferVec<SpriteVertex>(Pathfinder::BufferType::Vertex);
    }
};

struct C_SpriteBatch {
    C_AssetHandleWeak<Image> image_handle;
    GpuImage* gpu_image_handle = nullptr;

    bool operator==(const C_SpriteBatch& rhs) const {
        return image_handle == rhs.image_handle;
    }

    bool operator<(const C_SpriteBatch& rhs) const {
        return image_handle < rhs.image_handle;
    }
};

struct R_SpritePipeline {
    std::shared_ptr<Pathfinder::DescriptorSetLayout> descriptor_set_layout;
};

template <typename P>
class DrawSprite : public Draw<P> {
public:
    void prepare(const World& world) override{};

    void draw(const World& world, TrackedRenderPass& pass, Entity view_entity, P* item) override {
        auto& pipeline_cache = world.resource<std::shared_ptr<R_RenderPipelineCache>>();
        auto& descriptor_set_cache = world.resource<std::shared_ptr<R_DescriptorSetCache>>();
        auto& sprite_meta = world.resource<std::shared_ptr<R_SpriteMeta>>();

        auto entity = item->entity();
        auto& batch = world.registry.get<C_SpriteBatch>(entity);

        set_item_pipeline(item, pass, *pipeline_cache);

        set_descriptor_set(item, *sprite_meta, batch, *descriptor_set_cache, pass);
        draw_sprite_batch(item, *sprite_meta, pass, *pipeline_cache);
    };

private:
    Result<int, RenderCommandResult> set_item_pipeline(P* item,
                                                       TrackedRenderPass& pass,
                                                       R_RenderPipelineCache& pipeline_cache) {
        auto pipeline = pipeline_cache.get_render_pipeline(item->cached_pipeline());
        if (pipeline.has_value()) {
            pass.set_render_pipeline(item->cached_pipeline(), pipeline.value());
            return {RenderCommandResult::Success};
        } else {
            return {RenderCommandResult::Failure};
        }
    };

    Result<int, RenderCommandResult> set_descriptor_set(P* item,
                                                        const R_SpriteMeta& sprite_meta,
                                                        const C_SpriteBatch& sprite_batch,
                                                        R_DescriptorSetCache& descriptor_set_cache,
                                                        TrackedRenderPass& pass) {
        auto descriptor_set = descriptor_set_cache.get_descriptor_set(item->cached_descriptor_set());
        if (descriptor_set.has_value()) {
            pass.set_descriptor_set(item->cached_descriptor_set(), descriptor_set.value());
            return {RenderCommandResult::Success};
        } else {
            return {RenderCommandResult::Failure};
        }
    }

    Result<int, RenderCommandResult> draw_sprite_batch(P* item,
                                                       const R_SpriteMeta& sprite_meta,
                                                       TrackedRenderPass& pass,
                                                       R_RenderPipelineCache& pipeline_cache) {
        pass.set_vertex_buffer(0, {0}, 0, sprite_meta.vertices.get_buffer().value());
        pass.draw(item->batch_range().get().value(), {0, 1});
        return {RenderCommandResult::Success};
    };
};

void extract_sprites(World& world);
void queue_sprites(World& world);

} // namespace esr

#endif // ESR_SPRITE_RENDER_H
