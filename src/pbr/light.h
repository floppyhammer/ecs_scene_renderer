#ifndef ESR_PBR_LIGHT_H
#define ESR_PBR_LIGHT_H

#include <vector>

#include "../ecs/bundle.h"
#include "../ecs/entity.h"
#include "../render/color.h"
#include "../render/render.h"
#include "../render/render_resource/render_resource.h"
#include "../transform/components.h"

namespace esr {
    const float DEFAULT_SHADOW_DEPTH_BIA = 0.02;
    const float DEFAULT_SHADOW_NORMAL_BIAS = 0.6;

    struct C_PointLight {
        Color color = Color::white();
        float intensity;
        float range;
        float radius;
        bool shadows_enabled;
        float shadow_depth_bias;
        float shadow_normal_bias;
    };

    struct ExtractedPointLight {
        C_PointLight light;
        C_GlobalTransform transform;
    };

    struct C_SpotLight {
        Color color;
        float intensity;
        float range;
        float radius;
        bool shadows_enabled;
        float shadow_depth_bias;
        float shadow_normal_bias;
        float outer_angle;
        float inner_angle;
    };

    struct C_DirectionalLight {
        Color color;
        float illuminance;
        bool shadows_enabled;
        float shadow_depth_bias;
        float shadow_normal_bias;
    };

    struct R_AmbientLight {
        Color color = Color::white();
        float brightness = 0.05;
    };

    struct C_VisiblePointLights {
        std::vector<Entity> entities;
        uint32_t point_light_count;
        uint32_t spot_light_count;
    };

    struct R_ExtractedLights {
        std::vector<ExtractedPointLight> point_lights;
    };

    struct PointLightUniform {
        Vec3F position;
        float constant;
        Vec3F ambient;
        float linear;
        Vec3F diffuse;
        float quadratic;
        Vec3F specular;
        float padding0;
    };

    const uint32_t MAX_POINT_LIGHTS = 4;

    struct DirLightUniform {
        Vec3F direction = {0.0, -1.0, 0.0};
        float padding0;
        Vec3F ambient = Vec3F(0.03);
        float padding1;
        Vec3F diffuse = {1.0, 1.0, 1.0};
        float padding2;
        Vec3F specular;
        float padding3;
    };

    struct LightUniform {
        DirLightUniform dirLight;
        PointLightUniform pointLights[MAX_POINT_LIGHTS];
        uint32_t pointLightCount;
        uint32_t padding0;
        uint32_t padding1;
        uint32_t padding2;
    };

    struct R_LightUniform {
        std::shared_ptr<UniformBuffer<LightUniform>> uniform;
        std::shared_ptr<Pathfinder::Texture> shadow_map;
    };

    class World;

    class LightBundle : public Bundle {
    public:
        LightBundle() = default;

        Entity spawn(World &world) override;

        C_PointLight light;
        C_Transform transform;
        C_GlobalTransform global_transform;
        C_Visibility visibility;
    };

    void extract_lights(World &world);

    void prepare_light_uniform(World &world);

    void draw_shadow_map(World &world);
} // namespace esr

#endif // ESR_PBR_LIGHT_H
