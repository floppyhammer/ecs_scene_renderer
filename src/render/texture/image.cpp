#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace esr {

std::shared_ptr<GpuImage> Image::prepare_asset(R_RenderDevice& render_device, R_RenderQueue& render_queue) {
    auto driver = render_device.driver_;

    auto size = texture_descriptor.size;

    auto texture = driver->create_texture(texture_descriptor, "image_texture");

    if (!data.empty()) {
        auto command_encoder = render_queue.enqueue_command_buffer();
        command_encoder->write_texture(texture, Pathfinder::RectI{0, 0, size.x, size.y}, data.data());
    }

    auto sampler = driver->create_sampler(sampler_descriptor);

    auto render_pass = driver->create_render_pass(texture_descriptor.format, Pathfinder::AttachmentLoadOp::Clear, "image_render_pass");
    auto framebuffer = driver->create_framebuffer(render_pass, texture, "image_framebuffer");

    auto gpu_image = std::make_shared<GpuImage>();
    gpu_image->texture = texture;
    gpu_image->depth_texture = nullptr;
    gpu_image->texture_view = texture.get();
    gpu_image->texture_format = texture_descriptor.format;
    gpu_image->sampler = sampler;
    gpu_image->framebuffer = framebuffer;
    gpu_image->size = Vec2I{size.x, size.y};
    gpu_image->mip_level_count = 1;

    return gpu_image;
}

Result<std::shared_ptr<Image>, GenericError> Image::from_buffer(const uint8_t* data, size_t data_size) {
    stbi_set_flip_vertically_on_load(false);

    int32_t width, height, channels;
    unsigned char* img_data = stbi_load_from_memory(
        reinterpret_cast<const stbi_uc*>(data), data_size, &width, &height, &channels, STBI_rgb_alpha);

    if (!img_data) {
        return {GenericError::Error};
    }

    auto image = std::make_shared<Image>();
    image->data = {img_data, img_data + width * height * 4};
    image->texture_descriptor = {
        Pathfinder::Vec2I{width, height},
        Pathfinder::TextureFormat::Rgba8Unorm,
    };
    image->sampler_descriptor = {
        Pathfinder::SamplerFilter::Linear,
        Pathfinder::SamplerFilter::Linear,
        Pathfinder::SamplerAddressMode::Repeat,
        Pathfinder::SamplerAddressMode::Repeat,
    };

    stbi_image_free(img_data);

    return image;
}

std::shared_ptr<Image> Image::from_empty(Vec2I size) {
    auto image = std::make_shared<Image>();
    image->texture_descriptor = {
        Pathfinder::Vec2I{size.x, size.y},
        Pathfinder::TextureFormat::Rgba8Unorm,
    };
    image->sampler_descriptor = {
        Pathfinder::SamplerFilter::Linear,
        Pathfinder::SamplerFilter::Linear,
        Pathfinder::SamplerAddressMode::Repeat,
        Pathfinder::SamplerAddressMode::Repeat,
    };

    return image;
}

} // namespace esr
