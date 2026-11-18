#include "text_3d.h"

#include <stb/stb_image_write.h>
#include <xatlas/xatlas.h>

#include <codecvt>
#include <locale>

#include "../app/app.h"
#include "../ecs/schedule.h"
#include "../ecs/world.h"
#include "../hierarchy/child_builder.h"
#include "font.h"

namespace esr {

void unwrap_uv(std::shared_ptr<Mesh<Vertex>> mesh);

template <typename T>
std::string to_utf8(const std::basic_string<T, std::char_traits<T>, std::allocator<T>>& source) {
    std::wstring_convert<std::codecvt_utf8_utf16<T>, T> convertor;
    std::string result = convertor.to_bytes(source);

    return result;
}

template <typename T>
void from_utf8(const std::string& source, std::basic_string<T, std::char_traits<T>, std::allocator<T>>& result) {
    std::wstring_convert<std::codecvt_utf8_utf16<T>, T> convertor;
    result = convertor.from_bytes(source);
}

/// Create glyph entities in the main world.
void create_glyphs(World& world) {
    auto assets = world.resource<std::shared_ptr<R_Assets<Mesh<Vertex>>>>();

    auto query =
        world.registry
            .view<C_Text3d, C_AssetHandle<Text3dFont>, C_AssetHandle<StandardMaterial>, C_Transform, C_Visibility>();

    for (auto [entity, text, font, material_handle, transform, visibility] : query.each()) {
        Vec3F text_size;

        struct GlyphEntry {
            std::optional<C_AssetHandle<Mesh<Vertex>>> handle;
            float width = 0.0f;
        };

        std::vector<GlyphEntry> entries;

        std::u16string text_u16;
        from_utf8(text.content, text_u16);

        for (auto& c : text_u16) {
            if (c == u' ') {
                float space_width = 0.5f;
                entries.push_back({std::nullopt, space_width});
                text_size.x += space_width;
                continue;
            }

            auto iter = text.glyph_cache.find(c);
            if (iter != text.glyph_cache.end()) {
                auto& handle = iter->second;
                auto v = handle.get_data()->aabb->second - handle.get_data()->aabb->first;
                entries.push_back({handle, v.x});
                text_size.x += v.x;
                text_size.y = std::max(text_size.y, v.y);
                text_size.z = std::max(text_size.z, v.z);
            } else {
                auto mesh_opt = font.get_data()->generate_mesh(c, text.depth);
                if (!mesh_opt.has_value()) {
                    continue;
                }
                auto mesh = mesh_opt.value();
                auto mesh_handle = assets->add(mesh);
                text.glyph_cache[c] = mesh_handle;

                unwrap_uv(mesh);

                auto v = mesh->aabb->second - mesh->aabb->first;
                entries.push_back({mesh_handle, v.x});
                text_size.x += v.x;
                text_size.y = std::max(text_size.y, v.y);
                text_size.z = std::max(text_size.z, v.z);
            }
        }

        C_Transform glyph_transform;
        glyph_transform.translation.x = -text_size.x / 2;

        for (auto& entry : entries) {
            if (entry.handle.has_value()) {
                auto glyph_mesh_entity = world.spawn();
                world.add_component(glyph_mesh_entity, C_Text3dGlyph());
                world.add_component(glyph_mesh_entity, entry.handle.value());
                world.add_component(glyph_mesh_entity, material_handle);
                world.add_component(glyph_mesh_entity, glyph_transform);
                world.add_component(glyph_mesh_entity, C_GlobalTransform());
                world.add_component(glyph_mesh_entity, visibility);

                push_child(world, entity, glyph_mesh_entity);
            }
            glyph_transform.translation.x += entry.width;
        }
    }
}

void clear_glyphs(World& world) {
    auto query = world.registry.view<C_Text3d>();

    // Clear hierarchy relations before destroying entities.
    for (auto [entity, text] : query.each()) {
        clear_children(world, entity);
    }

    auto view = world.registry.view<C_Text3dGlyph>();
    world.registry.destroy(view.begin(), view.end());
}

void Text3dPlugin::build(App& app) {
    // Add AssetPlugin for font asset.
    app.add_plugin(std::make_shared<AssetPlugin<Text3dFont>>());

    app.add_system(std::make_shared<ScheduleLabelPreUpdate>(), clear_glyphs);
    app.add_system(std::make_shared<ScheduleLabelPostUpdate>(), create_glyphs);
}

Entity Text3dBundle::spawn(World& world) {
    auto entity = world.spawn();
    world.add_component(entity, text);
    world.add_component(entity, material);
    world.add_component(entity, transform);
    world.add_component(entity, global_transform);
    world.add_component(entity, font);
    world.add_component(entity, visibility);

    return entity;
}

static int Print(const char* format, ...) {
    va_list arg;
    va_start(arg, format);
    printf("\r"); // Clear progress text.
    const int result = vprintf(format, arg);
    va_end(arg);
    return result;
}

static void RandomColor(uint8_t* color) {
    for (int i = 0; i < 3; i++) {
        color[i] = uint8_t((rand() % 255 + 192) * 0.5f);
    }
}

static void SetPixel(uint8_t* dest, int destWidth, int x, int y, const uint8_t* color) {
    uint8_t* pixel = &dest[x * 3 + y * (destWidth * 3)];
    pixel[0] = color[0];
    pixel[1] = color[1];
    pixel[2] = color[2];
}

// https://github.com/miloyip/line/blob/master/line_bresenham.c
// License: public domain.
static void RasterizeLine(uint8_t* dest, int destWidth, const int* p1, const int* p2, const uint8_t* color) {
    const int dx = abs(p2[0] - p1[0]), sx = p1[0] < p2[0] ? 1 : -1;
    const int dy = abs(p2[1] - p1[1]), sy = p1[1] < p2[1] ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2;
    int current[2];
    current[0] = p1[0];
    current[1] = p1[1];
    while (SetPixel(dest, destWidth, current[0], current[1], color), current[0] != p2[0] || current[1] != p2[1]) {
        const int e2 = err;
        if (e2 > -dx) {
            err -= dy;
            current[0] += sx;
        }
        if (e2 < dy) {
            err += dx;
            current[1] += sy;
        }
    }
}

/*
https://github.com/ssloy/tinyrenderer/wiki/Lesson-2:-Triangle-rasterization-and-back-face-culling
Copyright Dmitry V. Sokolov

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it freely,
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If
you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not
required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original
software.
3. This notice may not be removed or altered from any source distribution.
*/
static void RasterizeTriangle(
    uint8_t* dest, int destWidth, const int* t0, const int* t1, const int* t2, const uint8_t* color) {
    if (t0[1] > t1[1]) std::swap(t0, t1);
    if (t0[1] > t2[1]) std::swap(t0, t2);
    if (t1[1] > t2[1]) std::swap(t1, t2);
    int total_height = t2[1] - t0[1];
    for (int i = 0; i < total_height; i++) {
        bool second_half = i > t1[1] - t0[1] || t1[1] == t0[1];
        int segment_height = second_half ? t2[1] - t1[1] : t1[1] - t0[1];
        float alpha = (float)i / total_height;
        float beta = (float)(i - (second_half ? t1[1] - t0[1] : 0)) / segment_height;
        int A[2], B[2];
        for (int j = 0; j < 2; j++) {
            A[j] = int(t0[j] + (t2[j] - t0[j]) * alpha);
            B[j] = int(second_half ? t1[j] + (t2[j] - t1[j]) * beta : t0[j] + (t1[j] - t0[j]) * beta);
        }
        if (A[0] > B[0]) std::swap(A, B);
        for (int j = A[0]; j <= B[0]; j++) SetPixel(dest, destWidth, j, t0[1] + i, color);
    }
}

void unwrap_uv(std::shared_ptr<Mesh<Vertex>> mesh) {
    const int MESH_COUNT = 1;

    xatlas::MeshDecl mesh_decl;
    mesh_decl.vertexCount = mesh->vertices.size();
    mesh_decl.vertexPositionData = mesh->vertices.data();
    mesh_decl.vertexPositionStride = sizeof(Vertex);
    mesh_decl.indexCount = mesh->indices->size();
    mesh_decl.indexData = mesh->indices->data();
    mesh_decl.indexFormat = xatlas::IndexFormat::UInt32;

    xatlas::Atlas* atlas = xatlas::Create();

    auto error = xatlas::AddMesh(atlas, mesh_decl, MESH_COUNT);
    if (error != xatlas::AddMeshError::Success) {
        xatlas::Destroy(atlas);
        printf("\rError unwrapping mesh uv: %s\n", xatlas::StringForEnum(error));
        return;
    }

    xatlas::ComputeCharts(atlas);

    xatlas::PackCharts(atlas);

    const xatlas::Mesh& x_mesh = atlas->meshes[0];
    assert(x_mesh.vertexCount == mesh->vertices.size());

    for (int i = 0; i < x_mesh.vertexCount; i++) {
        // Index of input vertex from which this output vertex originated.
        auto input_vertex = x_mesh.vertexArray[i].xref;
        auto& uv = x_mesh.vertexArray[i].uv;

        mesh->vertices[input_vertex].uv.x = uv[0] / atlas->width;
        mesh->vertices[input_vertex].uv.y = uv[1] / atlas->height;
    }

    // Debugging.
    //    if (atlas->width > 0 && atlas->height > 0) {
    //        printf("Rasterizing result...\n");
    //        // Dump images.
    //        std::vector<uint8_t> outputTrisImage, outputChartsImage;
    //        const uint32_t imageDataSize = atlas->width * atlas->height * 3;
    //        outputTrisImage.resize(atlas->atlasCount * imageDataSize);
    //        outputChartsImage.resize(atlas->atlasCount * imageDataSize);
    //        for (uint32_t i = 0; i < atlas->meshCount; i++) {
    //            const xatlas::Mesh& mesh = atlas->meshes[i];
    //            // Rasterize mesh triangles.
    //            const uint8_t white[] = {255, 255, 255};
    //            for (uint32_t j = 0; j < mesh.indexCount; j += 3) {
    //                int32_t atlasIndex = -1;
    //                bool skip = false;
    //                int verts[3][2];
    //                for (int k = 0; k < 3; k++) {
    //                    const xatlas::Vertex& v = mesh.vertexArray[mesh.indexArray[j + k]];
    //                    if (v.atlasIndex == -1) {
    //                        skip = true;
    //                        break;
    //                    }
    //                    atlasIndex = v.atlasIndex;
    //                    verts[k][0] = int(v.uv[0]);
    //                    verts[k][1] = int(v.uv[1]);
    //                }
    //                if (skip) continue; // Skip triangles that weren't atlased.
    //                uint8_t color[3];
    //                RandomColor(color);
    //                uint8_t* imageData = &outputTrisImage[atlasIndex * imageDataSize];
    //                RasterizeTriangle(imageData, atlas->width, verts[0], verts[1], verts[2], color);
    //                RasterizeLine(imageData, atlas->width, verts[0], verts[1], white);
    //                RasterizeLine(imageData, atlas->width, verts[1], verts[2], white);
    //                RasterizeLine(imageData, atlas->width, verts[2], verts[0], white);
    //            }
    //            // Rasterize mesh charts.
    //            for (uint32_t j = 0; j < mesh.chartCount; j++) {
    //                const xatlas::Chart* chart = &mesh.chartArray[j];
    //                uint8_t color[3];
    //                RandomColor(color);
    //                for (uint32_t k = 0; k < chart->faceCount; k++) {
    //                    int verts[3][2];
    //                    for (int l = 0; l < 3; l++) {
    //                        const xatlas::Vertex& v = mesh.vertexArray[mesh.indexArray[chart->faceArray[k] * 3 + l]];
    //                        verts[l][0] = int(v.uv[0]);
    //                        verts[l][1] = int(v.uv[1]);
    //                    }
    //                    uint8_t* imageData = &outputChartsImage[chart->atlasIndex * imageDataSize];
    //                    RasterizeTriangle(imageData, atlas->width, verts[0], verts[1], verts[2], color);
    //                    RasterizeLine(imageData, atlas->width, verts[0], verts[1], white);
    //                    RasterizeLine(imageData, atlas->width, verts[1], verts[2], white);
    //                    RasterizeLine(imageData, atlas->width, verts[2], verts[0], white);
    //                }
    //            }
    //        }
    //        for (uint32_t i = 0; i < atlas->atlasCount; i++) {
    //            char filename[256];
    //            snprintf(filename, sizeof(filename), "example_uvmesh_tris%02u.jpg", i);
    //            printf("Writing '%s'...\n", filename);
    //            stbi_write_jpg(filename, atlas->width, atlas->height, 3, &outputTrisImage[i * imageDataSize], 90);
    //            snprintf(filename, sizeof(filename), "example_uvmesh_charts%02u.jpg", i);
    //            printf("Writing '%s'...\n", filename);
    //            stbi_write_jpg(filename, atlas->width, atlas->height, 3, &outputChartsImage[i * imageDataSize], 90);
    //        }
    //    }

    xatlas::Destroy(atlas);
}

} // namespace esr
