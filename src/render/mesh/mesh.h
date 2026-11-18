#ifndef ESR_RENDER_MESH_H
#define ESR_RENDER_MESH_H

#include <cassert>
#include <optional>
#include <vector>

#include "../../app/plugin.h"
#include "../../geometry/geometry.h"
#include "../render_api.h"
#include "../render_resource/render_resource.h"
#include "vertex.h"

namespace esr {

class MeshPlugin : public Plugin {
    PLUGIN_INIT

    void build(App &app) override;
};

template <typename V>
class GpuMesh;

/// CPU mesh data.
template <typename V>
class Mesh {
public:
    Mesh() = default;

    std::vector<V> vertices;
    std::optional<std::vector<uint32_t>> indices;
    std::optional<std::pair<Vec3F, Vec3F>> aabb;

    std::string name;

    /// Converts an extracted mesh into a `GpuMesh`.
    std::shared_ptr<GpuMesh<V>> prepare_asset(R_RenderDevice &render_device, R_RenderQueue &render_queue) {
        auto driver = render_device.driver_;

        auto gpu_mesh = std::make_shared<GpuMesh<V>>(vertices, indices);

        gpu_mesh->upload(render_device, render_queue);

        return gpu_mesh;
    }
};

/// GPU data containing the mesh data (including a vertex buffer and an optional index buffer).
template <typename V>
class GpuMesh {
public:
    GpuMesh(const std::vector<V> &vertices, const std::optional<std::vector<uint32_t>> &indices) {
        buffer = BufferVec<Vertex>(Pathfinder::BufferType::Vertex);
        buffer.set_values(vertices);

        if (indices.has_value()) {
            assert(!indices.value().empty());

            index_buffer = BufferVec<uint32_t>(Pathfinder::BufferType::Index);
            index_buffer->set_values(indices.value());
        }
    }

    void update_data(const Mesh<V> &mesh) {
        buffer.set_values(mesh.vertices);

        if (mesh.indices.has_value()) {
            if (!index_buffer.has_value()) {
                index_buffer = BufferVec<uint32_t>(Pathfinder::BufferType::Index);
            }
            index_buffer->set_values(mesh.indices.value());
        }
    }

    std::shared_ptr<Pathfinder::Buffer> get_vertex_buffer() const {
        return buffer.get_buffer().value();
    }

    size_t get_vertex_count() const {
        return buffer.get_size();
    }

    std::optional<std::shared_ptr<Pathfinder::Buffer>> get_index_buffer() const {
        if (index_buffer.has_value()) {
            return index_buffer->get_buffer();
        }
        return {};
    }

    uint32_t get_index_count() const {
        if (index_buffer.has_value()) {
            return index_buffer->get_size();
        }
        return 0;
    }

    void upload(R_RenderDevice &device, R_RenderQueue &queue) {
        buffer.write_buffer(device, queue);

        if (index_buffer.has_value()) {
            index_buffer->write_buffer(device, queue);
        }
    }

private:
    BufferVec<Vertex> buffer;

    std::optional<BufferVec<uint32_t>> index_buffer;
};

} // namespace esr

#endif // ESR_RENDER_MESH_H
