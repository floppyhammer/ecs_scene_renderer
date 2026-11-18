#ifndef ESR_ASSET_H
#define ESR_ASSET_H

#include <iostream>
#include <map>
#include <string>

#include "../app/app.h"
#include "../app/main_schedule.h"
#include "../app/plugin.h"
#include "../ecs/world.h"
#include "../utils/result.h"
#include "asset_handle.h"

namespace esr {

class World;

Result<std::vector<uint8_t>, GenericError> load_file_as_bytes(const std::string& file_path);

/// Assets from the main world, in contrary to render assets from the render world.
///
/// In order to load assets into your game you must either
/// add them manually to an asset storage with Assets::add
/// or load them from the filesystem with Assets::load.
template <typename T>
class R_Assets {
public:
    /// Adds an asset to the collection, returning a strong handle to that asset.
    C_AssetHandle<T> add(std::shared_ptr<T> asset) {
        auto handle = C_AssetHandle(asset);
        assets[handle.get_id()] = handle;

        return handle;
    }

    /// Adds an asset to the collection, returning a strong handle to that asset.
    std::optional<C_AssetHandle<T>> load(const std::string& path) {
        auto id = AssetId::from_file(path);

        auto iter = assets.find(id);

        /// Asset has not been loaded.
        if (iter == assets.end()) {
            auto buffer = load_file_as_bytes(path);

            bool loading_is_ok = false;
            if (buffer.is_ok()) {
                auto data = T::from_buffer(buffer.unwrap().data(), buffer.unwrap().size());
                if (data.is_ok()) {
                    assets[id] = C_AssetHandle<T>(id, data.unwrap());
                    loading_is_ok = true;
                    std::cout << "Loaded asset: " << path << std::endl;
                }
            }

            if (!loading_is_ok) {
                return {};
            }
        }

        return assets[id];
    }

    /// Adds an asset to the collection, returning a strong handle to that asset.
    std::optional<C_AssetHandle<T>> load_from_memory(const uint8_t* data, size_t data_size) {
        auto id = AssetId::from_random();

        bool loading_is_ok = false;
        if (data && data_size > 0) {
            auto asset = T::from_buffer(data, data_size);
            if (asset.is_ok()) {
                assets[id] = C_AssetHandle<T>(id, asset.unwrap());
                loading_is_ok = true;
            }
        }

        if (!loading_is_ok) {
            return {};
        }

        return assets[id];
    }

    /// Gets a _Strong_ handle pointing to the same asset as the given one.
    std::optional<C_AssetHandle<T>> get_handle(AssetId id) {
        auto& iter = assets.find(id);

        if (iter == assets.end()) {
            return {};
        }

        return assets[id];
    }

    const std::map<AssetId, C_AssetHandle<T>>& iter() const {
        return assets;
    }

    std::map<AssetId, C_AssetHandle<T>>& iter_mut() {
        return assets;
    }

    void collect_garbage() {
        std::vector<AssetId> ids_to_erase;

        for (auto& pair : assets) {
            auto ref_count = pair.second.get_data().use_count();

            // One is in the storage, the other is in the loop pair.
            if (ref_count > 2) {
                continue;
            }

            ids_to_erase.push_back(pair.first);
        }

        for (auto& id : ids_to_erase) {
            assets.erase(id);
        }
    }

private:
    std::map<AssetId, C_AssetHandle<T>> assets;
};

template <typename T>
void free_assets(World& world) {
    auto asset_type = typeid(T).name();

    auto& assets = world.resource<std::shared_ptr<R_Assets<T>>>();

    assets->collect_garbage();
}

/// Adds support for `Assets` to an App.
///
/// Assets are typed collections with change tracking, which are added as App Resources. Examples of
/// assets: textures, sounds, 3d models, maps, scenes.
template <typename T>
class AssetPlugin : public Plugin {
    PLUGIN_INIT

    void build(App& app) override {
        app.insert_resource(std::make_shared<R_Assets<T>>());

        app.add_system(std::make_shared<ScheduleLabelFirst>(), free_assets<T>);
    }
};

} // namespace esr

#endif // ESR_ASSET_H
