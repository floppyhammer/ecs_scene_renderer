#include "font.h"

#include <vector>

namespace esr {

Result<std::shared_ptr<Text3dFont>, GenericError> Text3dFont::from_buffer(const uint8_t *data, size_t data_size) {
    auto font = std::make_shared<Text3dFont>();

    ttf_load_from_mem(data, data_size, &font->t2m_font, false);
    if (font->t2m_font == nullptr) {
        return {GenericError::Error};
    }

    return font;
}

Text3dFont::~Text3dFont() {
    ttf_free(t2m_font);
}

std::optional<std::shared_ptr<Mesh<Vertex>>> Text3dFont::generate_mesh(uint32_t unicode_char, float depth) {
    // Find a glyph in the font file.
    int index = ttf_find_glyph(t2m_font, unicode_char);
    if (index < 0) {
        return {};
    }

    // Make mesh object from the glyph.
    ttf_mesh3d_t *mesh;
    if (ttf_glyph2mesh3d(&t2m_font->glyphs[index], &mesh, TTF_QUALITY_NORMAL, TTF_FEATURES_DFLT, depth) != TTF_DONE) {
        return {};
    }

    ttf_glyph_t *glyph = &t2m_font->glyphs[index];

    auto glyph_mesh = std::make_shared<Mesh<Vertex>>();

    float min_x = 0, max_x = 0, min_y = 0, max_y = 0, min_z = 0, max_z = 0;

    for (int v = 0; v < mesh->nvert; v++) {
        glyph_mesh->vertices.push_back({{mesh->vert[v].x, mesh->vert[v].y, mesh->vert[v].z},
                                        {mesh->normals[v].x, mesh->normals[v].y, mesh->normals[v].z},
                                        {}});

        // Calculate AABB.
        min_x = std::min(min_x, mesh->vert[v].x);
        max_x = std::max(max_x, mesh->vert[v].x);

        min_y = std::min(min_y, mesh->vert[v].y);
        max_y = std::max(max_y, mesh->vert[v].y);

        min_z = std::min(min_z, mesh->vert[v].z);
        max_z = std::max(max_z, mesh->vert[v].z);
    }

    glyph_mesh->aabb = {{min_x, min_y, min_z}, {max_x, max_y, max_z}};

    if (mesh->nfaces > 0) {
        glyph_mesh->indices = std::vector<uint32_t>();

        for (int i = 0; i < mesh->nfaces; i++) {
            glyph_mesh->indices->push_back(mesh->faces[i].v1);
            glyph_mesh->indices->push_back(mesh->faces[i].v2);
            glyph_mesh->indices->push_back(mesh->faces[i].v3);
        }
    }

    ttf_free_mesh3d(mesh);

    return glyph_mesh;
}

} // namespace esr
