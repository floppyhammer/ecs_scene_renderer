#include "loader.h"

#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <assimp/Importer.hpp>
#include <unordered_map>

#include "../asset/asset.h"
#include "../ecs/world.h"
#include "../hierarchy/child_builder.h"
#include "../transform/transform.h"
#include "assimp/GltfMaterial.h"
#include "pbr.h"

namespace esr {

void AssimpPlugin::build(App &app) {
}

std::string split_directory(const std::string &str) {
    size_t found = str.find_last_of("/\\");

    //    std::cout << "Splitting: " << str << std::endl;
    //    std::cout << "Folder: " << str.substr(0, found) << std::endl;
    //    std::cout << "File: " << str.substr(found + 1) << std::endl;

    return str.substr(0, found);
}

std::vector<C_AssetHandle<Image>> AssimpLoader::load_material_textures(const aiScene *scene,
                                                                       aiMaterial *mat,
                                                                       aiTextureType type) {
    auto &image_assets = world->resource_mut<std::shared_ptr<R_Assets<Image>>>();

    std::vector<C_AssetHandle<Image>> images;

    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);

        std::optional<C_AssetHandle<Image>> image;
        std::string image_filename;

        auto tex_data = scene->GetEmbeddedTexture(str.C_Str());

        if (tex_data) {
            if (std::string(tex_data->achFormatHint) == "png") {
                image = image_assets->load_from_memory((uint8_t *)tex_data->pcData, tex_data->mWidth);
                image_filename = std::string(tex_data->mFilename.C_Str());
                std::cout << "Loaded material texture: " << image_filename << std::endl;
            } else {
                std::cout << "Embedded texture is not png!" << image_filename << std::endl;
            }
        } else {
            image_filename = directory + "/" + std::string(str.C_Str());

            image = image_assets->load(image_filename);
        }

        if (image.has_value()) {
            images.push_back(image.value());
        } else {
            std::cout << "Failed to load material image!" << image_filename << std::endl;
        }
    }
    return images;
}

void AssimpLoader::process_node(aiNode *node, const aiScene *scene, std::optional<Entity> parent) {
    // Get current node's local transform.
    Mat4 transform_matrix;
    std::memcpy(&transform_matrix, &node->mTransformation, sizeof(transform_matrix));
    C_Transform local_transform = C_Transform::from_matrix(transform_matrix.transpose());

    // Parent entity for all meshes of this node.
    auto node_entity = world->spawn();
    world->add_component(node_entity, C_GlobalTransform());
    world->add_component(node_entity, local_transform);
    if (parent.has_value()) {
        push_child(*world, parent.value(), node_entity);
    }

    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];

        auto mesh_handle = process_mesh(mesh, scene);

        MeshBundle bundle;
        bundle.mesh = mesh_handle;
        bundle.transform = local_transform;
        if (material_ids.back().second.has_value()) {
            bundle.material = materials[material_ids.back().second.value()];
        }

        auto mesh_entity = bundle.spawn(*world);
        push_child(*world, node_entity, mesh_entity);
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        process_node(node->mChildren[i], scene, node_entity);
    }
}

AlphaMode convert_alpha_mode(aiString alpha_mode) {
    if (strcmp(alpha_mode.C_Str(), "BLEND") == 0) {
        return AlphaMode::Blend;
    }
    if (strcmp(alpha_mode.C_Str(), "MASK") == 0) {
        return AlphaMode::Mask;
    }
    return AlphaMode::Opaque;
}

C_AssetHandle<Mesh<Vertex>> AssimpLoader::process_mesh(aiMesh *mesh, const aiScene *scene) {
    auto image_assets = world->resource<std::shared_ptr<R_Assets<Image>>>();
    auto mesh_assets = world->resource<std::shared_ptr<R_Assets<Mesh<Vertex>>>>();
    auto material_assets = world->resource<std::shared_ptr<R_Assets<StandardMaterial>>>();

    auto mesh_ = std::make_shared<Mesh<Vertex>>();

    mesh_->name = std::string(mesh->mName.C_Str());

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        vertex.pos = {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};

        //        if (mesh->HasVertexColors(0)) {
        //            vertex.color = {mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b};
        //        } else {
        //            vertex.color = {1.0, 1.0, 1.0};
        //        }

        if (mesh->HasNormals()) {
            vertex.normal = {mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z};
        } else {
            vertex.normal = {0.0, 0.0, 0.0};
        }

        if (mesh->HasTextureCoords(0)) {
            vertex.uv = {mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y};
        } else {
            vertex.uv = {0.0f, 0.0f};
        }

        mesh_->vertices.push_back(vertex);
    }

    // Handle faces (indices).
    if (mesh->HasFaces()) {
        mesh_->indices = std::vector<uint32_t>();

        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                mesh_->indices->push_back(face.mIndices[j]);
            }
        }
    }

    // Handle materials.
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
        auto base_color_images = load_material_textures(scene, material, aiTextureType_DIFFUSE);

        auto material_ = std::make_shared<StandardMaterial>();
        material_->name = std::string(material->GetName().C_Str());

        // Get opacity.
        // NOTE: we use the GLTF-specific macro AI_MATKEY_GLTF_ALPHAMODE here because AI_MATKEY_OPACITY does not work.
        aiString ai_alpha_mode;
        if (material->Get(AI_MATKEY_GLTF_ALPHAMODE, ai_alpha_mode) != AI_SUCCESS) {
            std::cout << "Failed to get AI_MATKEY_GLTF_ALPHAMODE!" << std::endl;
        }
        auto alpha_mode = convert_alpha_mode(ai_alpha_mode);
        material_->set_alpha_mode(alpha_mode);

        if (alpha_mode == AlphaMode::Mask) {
            float alpha_cutoff = 0.0;
            if (material->Get(AI_MATKEY_GLTF_ALPHACUTOFF, alpha_cutoff) != AI_SUCCESS) {
                std::cout << "Failed to get AI_MATKEY_GLTF_ALPHACUTOFF!" << std::endl;
            }
            material_->set_alpha_cutoff(alpha_cutoff);
        }

        if (!base_color_images.empty()) {
            material_->set_base_color_texture(base_color_images.front());
        }

        //        for (auto &image : base_color_images) {
        //            image_assets->add(image);
        //        }

        // New material handle.
        auto material_handle = material_assets->add(material_);

        materials[mesh->mMaterialIndex] = material_handle;

        material_ids.push_back(std::make_pair<uint32_t, std::optional<uint32_t>>(meshes.size(), mesh->mMaterialIndex));
    } else {
        material_ids.push_back(std::make_pair<uint32_t, std::optional<uint32_t>>(meshes.size(), {}));
    }

    // New mesh handle.
    auto mesh_handle = mesh_assets->add(mesh_);
    meshes.push_back(mesh_handle);

    return mesh_handle;
}

std::optional<Entity> AssimpLoader::load(std::string path) {
    meshes.clear();
    material_ids.clear();
    materials.clear();

    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return {};
    }

    directory = split_directory(path);

    Entity root_entity = world->spawn();
    world->add_component(root_entity, C_Transform());
    world->add_component(root_entity, C_GlobalTransform());

    process_node(scene->mRootNode, scene, root_entity);

    std::cout << "AssimpLoader loaded: " << path << ". Statistics: mesh count " << meshes.size() << " | material count "
              << materials.size() << std::endl;

    return root_entity;
}

} // namespace esr
