#ifndef ESR_RENDER_VERTEX_H
#define ESR_RENDER_VERTEX_H

#include <vector>

#include "../../geometry/vec3.h"
#include "../render_api.h"

namespace esr {

struct Vertex {
    Vec3F pos;
    Vec3F normal;
    Vec2F uv;

    bool operator==(const Vertex &other) const {
        return pos == other.pos && normal == other.normal && uv == other.uv;
    }

    template <class T1, class T2>
    std::size_t operator()(std::pair<T1, T2> const &v) const {
        return std::hash<T1>()(v.size());
    }

    /// Vertex attribute descriptions for Pathfinder.
    static std::vector<Pathfinder::VertexInputAttributeDescription> get_attribute_descriptions() {
        std::vector<Pathfinder::VertexInputAttributeDescription> attributes;
        attributes.reserve(3);

        attributes.push_back({0, 3, Pathfinder::DataType::f32, (uint32_t)sizeof(Vertex), offsetof(Vertex, pos), Pathfinder::VertexInputRate::Vertex});
        attributes.push_back({0, 3, Pathfinder::DataType::f32, (uint32_t)sizeof(Vertex), offsetof(Vertex, normal), Pathfinder::VertexInputRate::Vertex});
        attributes.push_back({0, 2, Pathfinder::DataType::f32, (uint32_t)sizeof(Vertex), offsetof(Vertex, uv), Pathfinder::VertexInputRate::Vertex});

        return attributes;
    }
};

} // namespace esr

#endif // ESR_RENDER_VERTEX_H
