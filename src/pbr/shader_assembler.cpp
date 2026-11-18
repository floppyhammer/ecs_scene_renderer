#include "shader_assembler.h"

#include "material.h"

namespace esr {

std::pair<std::shared_ptr<Pathfinder::ShaderModule>, std::shared_ptr<Pathfinder::ShaderModule>> create_mesh_shader_modules(
    Pathfinder::Device* driver, PipelineKey key) {
    bool has_base_color_texture = (key & PbrPipelineKeys::BASE_COLOR_TEXTURE).to_ulong() > 0;

    std::string vert_code = R"(
#version 450

layout (std140, binding = 1) uniform uView {
    mat4 viewProj;
    mat4 view;
    mat4 projection;
    vec3 cameraPosition;
    float pad0;
    vec4 viewport;
};

layout (std140, binding = 2) uniform uMesh {
    mat4 transform; // Model matrix
};

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoords;

layout(location = 0) out vec3 outWorldPos;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec2 outTexCoords;

void main() {
    vec4 worldPos = transform * vec4(inPos, 1.0);

    gl_Position = viewProj * worldPos;

    mat4 normalMatrix = view * transpose(inverse(transform));

    outWorldPos = worldPos.xyz;
    outNormal = (normalMatrix * vec4(inNormal, 1.0)).xyz;
    outTexCoords = inTexCoords;
}
)";

    bool is_unlit = (key & PbrPipelineKeys::UNLIT).to_ulong() > 0;

    std::string frag_code;
    if (is_unlit) {
        frag_code = R"(
#version 450

layout(location = 0) in vec3 WorldPos;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 TexCoords;

layout(location = 0) out vec4 outColor;

layout (std140, binding = 1) uniform uView {
    mat4 viewProj;
    mat4 view;
    mat4 projection;
    vec3 cameraPosition;
    float pad0;
    vec4 viewport;
};

layout(binding = 3) uniform uMaterial {
    vec4 baseColor;
    float metallic;
    float roughness;
    float ao;
    float alpha_cutoff;
};

struct PointLight {
    vec3 position;
    float constant;
    vec3 ambient;
    float linear;
    vec3 diffuse;
    float quadratic;
    vec3 specular;
    float padding0;
};

struct DirLight {
    vec3 direction;
    float padding0;
    vec3 ambient;
    float padding1;
    vec3 diffuse;
    float padding2;
    vec3 specular;
};

#define MAX_POINT_LIGHTS 4

layout(binding = 4) uniform uLight {
    DirLight dirLight;
    PointLight pointLights[MAX_POINT_LIGHTS];
    uint pointLightCount;
    uint lightPadding0;
    uint lightPadding1;
    uint lightPadding2;
};
)";

        if (has_base_color_texture) {
            frag_code += R"(
layout(binding = 5) uniform sampler2D albedoMap;

void main() {
    vec4 albedoMapColor = texture(albedoMap, TexCoords).rgba;
    vec4 albedo = vec4(pow(albedoMapColor.rgb * baseColor.rgb, vec3(2.2)), albedoMapColor.a * baseColor.a);
)";
        } else {
            frag_code += R"(
void main() {
    vec4 albedo = baseColor;
)";
        }

        frag_code += R"(
    vec3 color = albedo.rgb;

    // HDR tonemapping
    color = color / (color + vec3(1.0));

    // Gamma correction
    color = pow(color, vec3(1.0/2.2));
)";
    } else {
        frag_code = R"(
#version 450

layout(location = 0) in vec3 WorldPos;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 TexCoords;

layout(location = 0) out vec4 outColor;

layout (std140, binding = 1) uniform uView {
    mat4 viewProj;
    mat4 view;
    mat4 projection;
    vec3 cameraPosition;
    float pad0;
    vec4 viewport;
};

layout(binding = 3) uniform uMaterial {
    vec4 baseColor;
    float metallic;
    float roughness;
    float ao;
    float alpha_cutoff;
};

struct PointLight {
    vec3 position;
    float constant;
    vec3 ambient;
    float linear;
    vec3 diffuse;
    float quadratic;
    vec3 specular;
    float padding0;
};

struct DirLight {
    vec3 direction;
    float padding0;
    vec3 ambient;
    float padding1;
    vec3 diffuse;
    float padding2;
    vec3 specular;
};

#define MAX_POINT_LIGHTS 4

layout(binding = 4) uniform uLight {
    DirLight dirLight;
    PointLight pointLights[MAX_POINT_LIGHTS];
    uint pointLightCount;
    uint lightPadding0;
    uint lightPadding1;
    uint lightPadding2;
};

const float PI = 3.14159265359;

float DistributionGGX (vec3 N, vec3 H, float roughness) {
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX (float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith (vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 FresnelSchlick (float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow (1.0 - cosTheta, 5.0);
}
)";

        if (has_base_color_texture) {
            frag_code += R"(
layout(binding = 5) uniform sampler2D albedoMap;

void main() {
    vec4 albedoMapColor = texture(albedoMap, TexCoords).rgba;
    vec4 albedo = vec4(pow(albedoMapColor.rgb * baseColor.rgb, vec3(2.2)), albedoMapColor.a * baseColor.a);
)";
        } else {
            frag_code += R"(
void main() {
    vec4 albedo = baseColor;
)";
        }

        frag_code += R"(
    vec3 N = normalize(Normal);
    vec3 V = normalize(cameraPosition - WorldPos);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo.rgb, metallic);

    vec3 Lo = vec3(0.0);
    for (uint i = 0; i < min(pointLightCount, MAX_POINT_LIGHTS); ++i) {
        vec3 lightPos = pointLights[i].position;
        vec3 lightColor = pointLights[i].ambient;

        vec3 L = normalize(lightPos - WorldPos);
        vec3 H = normalize(V + L);
        float distance    = length(lightPos - WorldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance     = lightColor * attenuation;

        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);
        vec3 F    = FresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular     = numerator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        float NdotL = max(dot(N, L), 0.0);

        Lo += (kD * albedo.rgb / PI + specular) * radiance * NdotL;
    }

    vec3 lightDir = normalize(-dirLight.direction);
    float dirLightDiff = max(dot(Normal, lightDir), 0.0);

    vec3 ambient = dirLight.ambient * dirLight.diffuse * dirLightDiff * albedo.rgb * ao;

    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));

    // Gamma correction
    color = pow(color, vec3(1.0/2.2));
)";
    }

    if ((key & PbrPipelineKeys::ALPHA_MODE_MASK).to_ulong()) {
        std::string frag_code_alpha_mask = R"(
    if (albedo.a < alpha_cutoff) {
        discard;
    }

    outColor = vec4(color, 1.0);
}
)";
        frag_code += frag_code_alpha_mask;
    } else if ((key & PbrPipelineKeys::ALPHA_MODE_BLEND).to_ulong()) {
        std::string frag_code_alpha_blend = R"(
    outColor = vec4(color * albedo.a, albedo.a);
}
)";
        frag_code += frag_code_alpha_blend;
    } else {
        std::string frag_code_alpha_opaque = R"(
    outColor = vec4(color, 1.0);
}
)";
        frag_code += frag_code_alpha_opaque;
    }

    auto vert_translator = std::make_shared<Pathfinder::ShaderTranslator>(Pathfinder::ShaderStage::Vertex);
    auto frag_translator = std::make_shared<Pathfinder::ShaderTranslator>(Pathfinder::ShaderStage::Fragment);

    vert_translator->compile_from_glsl("main", vert_code, false);
    frag_translator->compile_from_glsl("main", frag_code, false);

    auto vert_shader_module = driver->create_shader_module(vert_translator->get_shader(), "pbr_vs");
    auto frag_shader_module = driver->create_shader_module(frag_translator->get_shader(), "pbr_fs");

    return {vert_shader_module, frag_shader_module};
}

} // namespace esr
