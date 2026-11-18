#include "render_app.h"

#include "render_schedule.h"

namespace esr {

void RenderApp::add_render_sub_graph(std::string sub_graph_name) {
    auto& render_graph = world->resource_mut<std::shared_ptr<RenderGraph>>();
    render_graph->add_sub_graph(sub_graph_name, RenderGraph());
}

void RenderApp::add_render_graph_edges(std::string sub_graph_name, std::vector<std::string> edges) {
    auto render_graph = world->resource_mut<std::shared_ptr<RenderGraph>>();

    auto res = render_graph->get_sub_graph_mut(sub_graph_name);
    if (res.has_value()) {
        res.value().get().add_node_edges(edges);
    } else {
        std::cout << "Tried adding render graph edges to a sub graph but the sub graph doesn't exist!" << std::endl;
    }
}

void RenderApp::add_render_graph_edge(std::string sub_graph_name, std::string output_edge, std::string input_edge) {
    auto render_graph = world->resource_mut<std::shared_ptr<RenderGraph>>();

    auto res = render_graph->get_sub_graph_mut(sub_graph_name);
    if (res.has_value()) {
        res.value().get().add_node_edge(NodeLabel::from_name(output_edge), NodeLabel::from_name(input_edge));
    } else {
        std::cout << "Tried adding render graph edges to a sub graph but the sub graph doesn't exist!" << std::endl;
    }
}

} // namespace esr
