#include "mesh.h"

#include "../../app/app.h"
#include "../../asset/asset.h"

namespace esr {

void MeshPlugin::build(App& app) {
    app.add_plugin(std::make_shared<AssetPlugin<Mesh<Vertex>>>());
}

} // namespace esr
