#ifndef ESR_CORE_PIPELINE_H
#define ESR_CORE_PIPELINE_H

#include "../app/plugin.h"

namespace esr {

class CorePipelinePlugin : public Plugin {
    PLUGIN_INIT

    void build(App& app) override;
};

} // namespace esr

#endif // ESR_CORE_PIPELINE_H
