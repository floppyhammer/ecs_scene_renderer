#include "edge.h"

#include "graph.h"

namespace esr {

NodeId Edge::get_input_node() const {
    if (type == Type::SlotEdge) {
        return std::get<SlotEdge>(var).input_node;
    } else {
        return std::get<NodeEdge>(var).input_node;
    }
}

NodeId Edge::get_output_node() const {
    if (type == Type::SlotEdge) {
        return std::get<SlotEdge>(var).output_node;
    } else {
        return std::get<NodeEdge>(var).output_node;
    }
}

bool Edges::has_input_edge(const Edge& edge) const {
    return std::find(input_edges.begin(), input_edges.end(), edge) != input_edges.end();
}

bool Edges::has_output_edge(const Edge& edge) const {
    return std::find(output_edges.begin(), output_edges.end(), edge) != output_edges.end();
}

Result<int, RenderGraphError> Edges::add_input_edge(const Edge& edge) {
    if (has_input_edge(edge)) {
        return {RenderGraphError::EdgeAlreadyExists};
    }
    input_edges.push_back(edge);
    return {0};
}

Result<int, RenderGraphError> Edges::add_output_edge(const Edge& edge) {
    if (has_output_edge(edge)) {
        return {RenderGraphError::EdgeAlreadyExists};
    }
    output_edges.push_back(edge);
    return {0};
}

} // namespace esr
