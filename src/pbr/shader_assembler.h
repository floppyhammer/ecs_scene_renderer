#ifndef ESR_PBR_SHADER_ASSEMBLER_H
#define ESR_PBR_SHADER_ASSEMBLER_H

#include "../render/render_api.h"
#include "../render/render_resource/pipeline_specializer.h"
#include "pathfinder/gpu/device.h"
#include "pathfinder/gpu/shader_translator.h"

namespace esr {

std::pair<std::shared_ptr<Pathfinder::ShaderModule>, std::shared_ptr<Pathfinder::ShaderModule>>
create_mesh_shader_modules(Pathfinder::Device* driver, PipelineKey key);

}

#endif // ESR_PBR_SHADER_ASSEMBLER_H
