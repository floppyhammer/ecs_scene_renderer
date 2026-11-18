#ifndef ESR_RENDER_IMAGE_H
#define ESR_RENDER_IMAGE_H

#include <cstdint>
#include <optional>
#include <vector>

#include "../../asset/asset.h"
#include "../../geometry/vec2.h"
#include "../../utils/result.h"
#include "../render_api.h"

namespace esr {

struct GpuImage;

/// An error that occurs when loading a texture.
enum class TextureError {
    InvalidImageExtension,
    ImageError,
    UnsupportedTextureFormat,
    InvalidData,
};

struct Image {
    std::vector<uint8_t> data;

    Pathfinder::TextureDescriptor texture_descriptor;
    Pathfinder::SamplerDescriptor sampler_descriptor;

    static std::shared_ptr<Image> from_empty(Vec2I size);

    static Result<std::shared_ptr<Image>, GenericError> from_buffer(const uint8_t* data, size_t data_size);

    /// Create a `GpuImage` from the image.
    std::shared_ptr<GpuImage> prepare_asset(R_RenderDevice& render_device, R_RenderQueue& render_queue);
};

/// The GPU-representation of an `Image`.
/// Consists of the `Texture`, its `Sampler`, and the texture's size.
struct GpuImage {
    void update_data(const Image& image) {
        // TODO
    }

    void upload(R_RenderDevice& device, R_RenderQueue& queue) {
        // TODO
    }

    std::shared_ptr<Pathfinder::Texture> texture;
    std::shared_ptr<Pathfinder::Texture> depth_texture;
    Pathfinder::Texture* texture_view;
    Pathfinder::TextureFormat texture_format;
    std::shared_ptr<Pathfinder::Sampler> sampler;
    std::shared_ptr<Pathfinder::Framebuffer> framebuffer;
    Vec2I size;
    uint32_t mip_level_count;
};

} // namespace esr

#endif // ESR_RENDER_IMAGE_H
