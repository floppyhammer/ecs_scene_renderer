#ifndef ESR_RENDER_ASSET_H
#define ESR_RENDER_ASSET_H

#include <map>

#include "../app/app.h"
#include "../app/plugin.h"
#include "../asset/asset.h"
#include "../asset/asset_handle.h"
#include "../utils/result.h"
#include "render.h"
#include "render_schedule.h"

namespace esr {

class App;

// In ms.
const uint32_t RENDER_ASSET_DECAY_TIME = 400;

/// Describes how an asset gets extracted and prepared for rendering.
///
/// In the `ExtractSchedule` step the asset is transferred
/// from the "main world" into the "render world".
/// Therefore it is converted into a `RenderAsset::ExtractedAsset`, which may be the same type
/// as the render asset itself.
///
/// After that in the `RenderSet::Prepare` step the extracted asset
/// is transformed into its GPU-representation of type `RenderAsset::PreparedAsset`.
template <typename E, typename P>
class R_RenderAssets {
public:
    R_RenderAssets() {
        cpu_asset_name = typeid(E).name();
        gpu_asset_name = typeid(P).name();

        start_time = std::chrono::steady_clock::now();
    }

    std::optional<std::shared_ptr<P>> get_gpu_asset(const C_AssetHandleWeak<E>& key) {
        auto iter = storage.find(key);
        if (iter != storage.end()) {
            last_time_used[key] = std::chrono::steady_clock::now();

            return iter->second;
        }
        return {};
    }

    bool is_prepared(const C_AssetHandleWeak<E>& key) const {
        return storage.find(key) != storage.end();
    }

    void push(const C_AssetHandleWeak<E>& key, std::shared_ptr<P> gpu_asset) {
        storage[key] = gpu_asset;
        last_time_used[key] = std::chrono::steady_clock::now();
    }

    /// Free unused GPU assets.
    void collect_garbage() {
        std::vector<C_AssetHandleWeak<E>> assets_to_remove;

        for (auto& pair : storage) {
            auto& handle = pair.first;

            std::chrono::duration<double> elapsed_time = std::chrono::steady_clock::now() - last_time_used[handle];

            auto elapsed_time_in_ms = elapsed_time.count() * 1000.f;

            if (elapsed_time_in_ms > RENDER_ASSET_DECAY_TIME) {
                assets_to_remove.push_back(handle);

                std::cout << "Render asset (" << handle.get_type() << ") [" << handle.get_id()
                          << "] freed after being idle for " << elapsed_time_in_ms << " ms." << std::endl;
            }
        }

        for (auto& handle : assets_to_remove) {
            remove(handle);
        }
    }

private:
    bool remove(const C_AssetHandleWeak<E>& key) {
        auto iter = storage.find(key);

        if (iter != storage.end()) {
            storage.erase(key);
            last_time_used.erase(key);
            return true;
        }

        return false;
    }

private:
    std::map<C_AssetHandleWeak<E>, std::shared_ptr<P>> storage;
    std::map<C_AssetHandleWeak<E>, std::chrono::time_point<std::chrono::steady_clock>> last_time_used;

    std::chrono::time_point<std::chrono::steady_clock> start_time;

    // For debugging.
    std::string cpu_asset_name;
    std::string gpu_asset_name;
};

/// Temporarily stores the extracted and removed assets of the current frame.
template <typename A>
struct R_ExtractedAssets {
    std::vector<C_AssetHandleWeak<A>> extracted;
    std::vector<C_AssetHandleWeak<A>> modified;
    std::vector<C_AssetHandleWeak<A>> removed;

    void clear_all() {
        extracted.clear();
        modified.clear();
        removed.clear();
    }
};

/// This system extracts all created or modified assets of the corresponding `RenderAsset` type
/// into the "render world".
template <typename A>
void extract_render_asset(World& world) {
    auto asset_type = typeid(A).name();

    auto main_world = world.resource<std::shared_ptr<R_MainWorld>>()->world_;

    auto assets = main_world->resource<std::shared_ptr<R_Assets<A>>>();

    auto& extracted_assets = world.resource_mut<std::shared_ptr<R_ExtractedAssets<A>>>();
    extracted_assets->clear_all();

    for (auto& asset : assets->iter_mut()) {
        auto weak_handle = asset.second.as_weak();
        extracted_assets->extracted.push_back(weak_handle);

        if (asset.second.is_modified()) {
            extracted_assets->modified.push_back(weak_handle);
            asset.second.mark_as_modified(false);
        }

        //        if (asset.second.is_removed()) {
        //            extracted_assets->removed.push_back(weak_handle);
        //        }
    }
}

/// This system prepares all assets of the corresponding `RenderAsset` type
/// which where extracted this frame for the GPU.
template <typename E, typename P>
void prepare_render_assets(World& world) {
    auto cpu_asset_type = typeid(E).name();
    auto gpu_asset_type = typeid(P).name();

    auto& extracted_assets = world.resource_mut<std::shared_ptr<R_ExtractedAssets<E>>>();
    auto& prepared_assets = world.resource_mut<std::shared_ptr<R_RenderAssets<E, P>>>();

    auto& render_device = world.resource_mut<std::shared_ptr<R_RenderDevice>>();
    auto& render_queue = world.resource_mut<std::shared_ptr<R_RenderQueue>>();

    // Prepare GPU assets.
    for (C_AssetHandleWeak<E>& extracted : extracted_assets->extracted) {
        if (!prepared_assets->is_prepared(extracted)) {
            auto gpu_asset = extracted.get_data()->prepare_asset(*render_device, *render_queue);
            prepared_assets->push(extracted, gpu_asset);
        }
    }

    // Update modified GPU assets.
    for (auto& modified : extracted_assets->modified) {
        auto gpu_asset = prepared_assets->get_gpu_asset(modified).value();
        gpu_asset->update_data(*modified.get_data());
        gpu_asset->upload(*render_device, *render_queue);
    }

    // Remove unused GPU assets.
    //    for (auto& removed : extracted_assets->removed) {
    //        bool successful = prepared_assets->remove(removed);
    //    }
}

template <typename E, typename P>
void free_render_assets(World& world) {
    auto cpu_asset_type = typeid(E).name();
    auto gpu_asset_type = typeid(P).name();

    auto& prepared_assets = world.resource_mut<std::shared_ptr<R_RenderAssets<E, P>>>();

    prepared_assets->collect_garbage();
}

template <typename E, typename P>
class RenderAssetPlugin : public Plugin {
    PLUGIN_INIT

    void build(App& app) {
        auto res = app.get_sub_app("Render");
        if (!res.is_ok()) {
            return;
        }

        auto render_app = res.unwrap();

        // Track the CPU representation of the asset.
        render_app->insert_resource(std::make_shared<R_ExtractedAssets<E>>());
        // Track the GPU representation of the asset.
        render_app->insert_resource(std::make_shared<R_RenderAssets<E, P>>());

        // Extract the CPU representation of the asset from the main world.
        render_app->add_system(std::make_shared<ScheduleLabelExtract>(), extract_render_asset<E>);
        // Prepare the GPU representation of the asset using the CPU counterpart.
        render_app->add_system(std::make_shared<ScheduleLabelPrepare>(), prepare_render_assets<E, P>);
        render_app->add_system(std::make_shared<ScheduleLabelCleanup>(), free_render_assets<E, P>);
    }
};

} // namespace esr

#endif // ESR_RENDER_ASSET_H
