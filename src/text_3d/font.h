#ifndef ESR_FONT_H
#define ESR_FONT_H

#include <ttf2mesh/ttf2mesh.h>

#include <cstdio>
#include <cstdlib>

#include "../asset/asset.h"
#include "../asset/asset_handle.h"
#include "../render/mesh/mesh.h"
#include "../render/mesh/vertex.h"
#include "../utils/result.h"

namespace esr {

class Text3dFont {
public:
    explicit Text3dFont() = default;

    static Result<std::shared_ptr<Text3dFont>, GenericError> from_buffer(const uint8_t* data, size_t data_size);

    ~Text3dFont();

    /// The generated mesh has the same baseline and origin as in 2D.
    std::optional<std::shared_ptr<Mesh<Vertex>>> generate_mesh(uint32_t unicode_char, float depth);

private:
    ttf_t* t2m_font = nullptr;
};

} // namespace esr

#endif // ESR_FONT_H
