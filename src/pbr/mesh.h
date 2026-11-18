#ifndef ESR_PBR_MESH_H
#define ESR_PBR_MESH_H

#include "../app/app.h"
#include "../app/plugin.h"
#include "../geometry/mat4.h"
#include "../render/mesh/mesh.h"
#include "../render/render_phase/draw.h"
#include "../render/render_phase/draw_state.h"
#include "../render/render_resource/pipeline_cache.h"
#include "../render/view/view.h"
#include "light.h"
#include "material.h"

namespace esr {

class StandardMaterial;

class MeshRenderPlugin : public Plugin {
    PLUGIN_INIT

    void build(App& app) override;
};

class ExtractedMesh {
public:
    C_AssetHandleWeak<Mesh<Vertex>> handle;
    std::optional<C_AssetHandleWeak<StandardMaterial>> material;
    Mat4 transform;
};

class R_ExtractedMeshes {
public:
    std::vector<ExtractedMesh> meshes;
};

struct MeshUniforms {
    Mat4 transform;
};

struct C_RenderableMesh {
    GpuMesh<Vertex>* gpu_mesh;
    StandardMaterial material;
};

struct R_MeshUniformStorage {
    std::vector<std::shared_ptr<UniformBuffer<MeshUniforms>>> buffers;
};

struct PbrDescriptorSetKey {
    StandardMaterial material;
    uint32_t view_offset;
    std::shared_ptr<UniformBuffer<MeshUniforms>> mesh_uniform;
    std::shared_ptr<UniformBuffer<MaterialUniform>> material_uniform;
    std::shared_ptr<UniformBuffer<LightUniform>> light_uniform;

    inline bool operator<(const PbrDescriptorSetKey& rhs) const {
        if (material < rhs.material || rhs.material < material) {
            return material < rhs.material;
        } else if (view_offset != rhs.view_offset) {
            return view_offset < rhs.view_offset;
        } else if (mesh_uniform != rhs.mesh_uniform) {
            return mesh_uniform < rhs.mesh_uniform;
        } else if (material_uniform != rhs.material_uniform) {
            return material_uniform < rhs.material_uniform;
        } else {
            return light_uniform < rhs.light_uniform;
        }
    }
};

template <typename P>
class DrawMesh : public Draw<P> {
public:
    void prepare(const World& world) override{};

    void draw(const World& world, TrackedRenderPass& pass, Entity view_entity, P* item) override {
        auto& pipeline_cache = world.resource<std::shared_ptr<R_RenderPipelineCache>>();
        auto& descriptor_set_cache = world.resource<std::shared_ptr<R_DescriptorSetCache>>();

        auto& mesh = world.registry.get<C_RenderableMesh>(item->entity());

        auto pipeline = pipeline_cache->get_render_pipeline(item->cached_pipeline());
        if (pipeline.has_value()) {
            pass.set_render_pipeline(item->cached_pipeline(), pipeline.value());
        } else {
            abort();
        }

        pass.set_vertex_buffer(0, BufferId{0}, 0, mesh.gpu_mesh->get_vertex_buffer());
        if (mesh.gpu_mesh->get_index_buffer().has_value()) {
            pass.set_index_buffer(
                BufferId{0}, 0, Pathfinder::DataType::u32, mesh.gpu_mesh->get_index_buffer().value());
        }

        auto descriptor_set = descriptor_set_cache->get_descriptor_set(item->cached_descriptor_set());
        if (descriptor_set.has_value()) {
            pass.set_descriptor_set(item->cached_descriptor_set(), descriptor_set.value());
        } else {
            abort();
        }

        if (mesh.gpu_mesh->get_index_buffer().has_value()) {
            pass.draw_indexed(
                Range{0, mesh.gpu_mesh->get_vertex_count()}, Range{0, mesh.gpu_mesh->get_index_count()}, {0, 1});
        } else {
            pass.draw(Range{0, mesh.gpu_mesh->get_vertex_count()}, {0, 1});
        }
    };
};

} // namespace esr

#endif // ESR_PBR_MESH_H
