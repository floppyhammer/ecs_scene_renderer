#ifndef ESR_TRANSFORM_H
#define ESR_TRANSFORM_H

#include "../app/plugin.h"
#include "components.h"

namespace esr {

class App;

/// A `Bundle` of the `Transform` and `GlobalTransform`
/// components, which describe the position of an entity.
struct TransformBundle {
    /// The transform of the entity.
    C_Transform local;
    /// The global transform of the entity.
    C_GlobalTransform global;
};

class TransformPlugin : public Plugin {
    PLUGIN_INIT

    void build(App& app) override;
};

} // namespace esr

#endif // ESR_TRANSFORM_H
