#ifndef ESR_PBR_MATERIAL_H
#define ESR_PBR_MATERIAL_H

#include <iostream>
#include <memory>

#include "../app/plugin.h"
#include "../render/../asset/asset_handle.h"
#include "../render/color.h"
#include "../render/render_asset.h"
#include "../render/render_resource/pipeline_specializer.h"
#include "../render/render_resource/render_resource.h"
#include "../render/texture/image.h"

namespace esr {

class App;

struct PbrPipelineKeys {
    static const PipelineKey BASE_COLOR_TEXTURE;
    static const PipelineKey NORMAL_TEXTURE;
    static const PipelineKey ALPHA_MODE_OPAQUE;
    static const PipelineKey ALPHA_MODE_BLEND;
    static const PipelineKey ALPHA_MODE_MASK;
    static const PipelineKey UNLIT;
};

enum class Face {
    /// Front ALPHA_MODE_OPAQUE
    Front = 0,
    /// Back face
    Back = 1,
};

enum class AlphaMode {
    Opaque = 0,
    Blend,
    Mask,
};

/// This is used to cache material pipelines.
///
/// Materials with different image slots will generate different pipelines.
/// With the same pipeline layout, different image bindings in the those slots will generate different descriptor sets.
class StandardMaterialPipelineLayout {
public:
    bool base_color_texture;
    bool normal_texture;
    AlphaMode alpha_mode;
    bool unlit;

    PipelineKey get_pipeline_key() const {
        PipelineKey key;
        if (base_color_texture) {
            key |= PbrPipelineKeys::BASE_COLOR_TEXTURE;
        }
        if (normal_texture) {
            key |= PbrPipelineKeys::NORMAL_TEXTURE;
        }

        switch (alpha_mode) {
            case AlphaMode::Opaque: {
                key |= PbrPipelineKeys::ALPHA_MODE_OPAQUE;
            } break;
            case AlphaMode::Blend: {
                key |= PbrPipelineKeys::ALPHA_MODE_BLEND;
            } break;
            case AlphaMode::Mask: {
                key |= PbrPipelineKeys::ALPHA_MODE_MASK;
            } break;
            default:
                abort();
        }

        if (unlit) {
            key |= PbrPipelineKeys::UNLIT;
        }

        return key;
    }

    bool operator<(const StandardMaterialPipelineLayout& rhs) const {
        return get_pipeline_key().to_ulong() < rhs.get_pipeline_key().to_ulong();
    }
};

/// `Handle` is used as map key to cache materials in CPU, but in GPU,
/// `StandardMaterial` is used instead.
/// Since we might have many unique identical materials (e.g. empty materials),
/// we definitely don't want to create a `DescriptorSet` for each of them.
class StandardMaterial {
public:
    StandardMaterial() = default;

    void set_base_color_texture(C_AssetHandle<Image> new_texture);

    std::optional<C_AssetHandle<Image>> get_base_color_texture() const;

    StandardMaterialPipelineLayout get_pipeline_layout() const;

    std::shared_ptr<Pathfinder::RenderPipeline> generate_pipeline();

    std::shared_ptr<Pathfinder::DescriptorSet> generate_descriptor_set();

    bool operator<(const StandardMaterial& rhs) const {
        if (alpha_mode_ != rhs.alpha_mode_) {
            return alpha_mode_ < rhs.alpha_mode_;
        }
        if (base_color_texture.has_value() && rhs.base_color_texture.has_value()) {
            return base_color_texture.value() < rhs.base_color_texture.value();
        }
        if (base_color_texture.has_value()) {
            return true;
        }
        return false;
    }

    std::string name;

    void set_alpha_mode(AlphaMode alpha_mode) {
        alpha_mode_ = alpha_mode;
    }

    AlphaMode get_alpha_mode() const {
        return alpha_mode_;
    }

    void set_alpha_cutoff(float alpha_cutoff) {
        alpha_cutoff_ = {alpha_cutoff};
    }

    std::optional<float> get_alpha_cutoff() const {
        return alpha_cutoff_;
    }

    Color get_base_color() const {
        return base_color;
    }

    float get_metallic() const {
        return metallic;
    }

    float get_roughness() const {
        return perceptual_roughness;
    }

    float get_ao() const {
        return 0.5;
    }

private:
    /// The color of the surface of the material before lighting.
    Color base_color = Color::white();

    /// The texture component of the material's color before lighting.
    /// The actual pre-lighting color is `base_color * this_texture`.
    std::optional<C_AssetHandle<Image>> base_color_texture;

    /// Color the material "emits" to the camera.
    Color emissive = Color::white();

    /// The emissive map, multiplies pixels with `emissive`
    /// to get the final "emitting" color of a surface.
    std::optional<C_AssetHandle<Image>> emissive_texture;

    /// Linear perceptual roughness, clamped to [0.089, 1.0] in the shader.
    float perceptual_roughness = 0.5;

    /// How "metallic" the material appears, within `[0.0, 1.0]`.
    ///
    /// If used together with a roughness/metallic texture, this is factored into the final base
    /// color as `metallic * metallic_texture_value`.
    float metallic = 0.0;

    /// Metallic and roughness maps, stored as a single texture.
    std::optional<C_AssetHandle<Image>> metallic_roughness_texture;

    /// Specular intensity for non-metals on a linear scale of [0.0, 1.0].
    float reflectance = 0.5;

    /// Used to fake the lighting of bumps and dents on a material.
    std::optional<C_AssetHandle<Image>> normal_map_texture;

    /// Specifies the level of exposure to ambient light.
    std::optional<C_AssetHandle<Image>> occlusion_texture;

    Face cull_mode = Face::Back;

    AlphaMode alpha_mode_;

    /// Only for AlphaMode::Mask.
    std::optional<float> alpha_cutoff_;

    /// Whether to apply only the base color (and the color texture) to this material. (No PBR)
    bool unlit_ = false;
};

struct MaterialUniform {
    float base_color[4];
    float metallic;
    float roughness;
    float ao;
    float alpha_cutoff;
};

/// Buffers to store the material uniforms.
struct R_MaterialUniformStorage {
    std::vector<std::shared_ptr<UniformBuffer<MaterialUniform>>> buffers;
    /// For meshes without a material.
    std::shared_ptr<UniformBuffer<MaterialUniform>> default_buffer;
};

class MaterialPlugin : public Plugin {
    PLUGIN_INIT

    void build(App& app) override;
};

struct R_ExtractedMaterials {
    std::vector<C_AssetHandleWeak<StandardMaterial>> extracted;
    std::vector<C_AssetHandleWeak<StandardMaterial>> removed;
};

struct R_PbrPipeline {
    int _;
};

} // namespace esr

#endif // ESR_PBR_MATERIAL_H
