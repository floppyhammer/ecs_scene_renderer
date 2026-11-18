#ifndef ESR_RENDER_TEXTURE_CACHE_H
#define ESR_RENDER_TEXTURE_CACHE_H

#include <map>
#include <memory>
#include <vector>

#include "../render_api.h"

namespace esr {

struct CachedTexture {
    std::shared_ptr<Pathfinder::Texture> texture;
    Pathfinder::Texture* default_view;
};

struct CachedTextureMeta {
    std::shared_ptr<Pathfinder::Texture> texture;
    Pathfinder::Texture* default_view;
    bool taken;
    size_t frames_since_last_use;
};

struct R_TextureCache {
    std::map<Pathfinder::TextureDescriptor, std::vector<CachedTextureMeta>> textures;
};

} // namespace esr

#endif // ESR_RENDER_TEXTURE_CACHE_H
