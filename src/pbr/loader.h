#ifndef ESR_PBR_LOADER_H
#define ESR_PBR_LOADER_H

#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <array>
#include <assimp/Importer.hpp>
#include <string>
#include <utility>
#include <vector>

#include "../asset/asset.h"
#include "../ecs/bundle.h"
#include "../ecs/world.h"
#include "../render/render.h"
#include "material.h"

namespace esr {

class AssimpPlugin : public Plugin {
    PLUGIN_INIT

    void build(App &app) override;
};

class AssimpLoader {
public:
    AssimpLoader(World *world_) {
        world = world_;
    }

    std::optional<Entity> load(std::string path);

private:
    std::vector<C_AssetHandle<Mesh<Vertex>>> meshes;

    /// Material id -> material.
    std::map<uint32_t, C_AssetHandle<StandardMaterial>> materials;

    /// Mesh id -> material id.
    std::vector<std::pair<uint32_t, std::optional<uint32_t>>> material_ids;

    World *world;

    /// Directory the model file located.
    std::string directory;

    void process_node(aiNode *node, const aiScene *scene, std::optional<Entity> parent);

    C_AssetHandle<Mesh<Vertex>> process_mesh(aiMesh *mesh, const aiScene *scene);

    std::vector<C_AssetHandle<Image>> load_material_textures(const aiScene *scene, aiMaterial *mat, aiTextureType type);
};

} // namespace esr

#endif // ESR_PBR_LOADER_H
