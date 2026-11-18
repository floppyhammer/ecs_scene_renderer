#ifndef ESR_RENDER_APP_H
#define ESR_RENDER_APP_H

#include <string>
#include <vector>

#include "../app/app.h"

namespace esr {

/// Adds common `RenderGraph` operations to `App`.
class RenderApp : public App {
public:
    /// Never call this constructor directly. Use static member functions below.
    RenderApp() = default;

    /// Add a sub graph to the `RenderGraph`.
    void add_render_sub_graph(std::string sub_graph_name);

    /// Add a `Node` to the `RenderGraph`:
    /// * Create the `Node` using the `FromWorld` implementation.
    /// * Add it to the graph.
    template <typename T>
    void add_render_graph_node(std::string sub_graph_name, std::string node_name) {
        auto node = std::make_shared<T>();
        auto render_graph = world->resource_mut<std::shared_ptr<RenderGraph>>();

        auto res = render_graph->get_sub_graph_mut(sub_graph_name);
        if (res.has_value()) {
            res.value().get().add_node(node_name, node);
        } else {
            std::cout << "Tried adding a render graph node to a sub graph but the sub graph doesn't exist!"
                      << std::endl;
        }
    }

    /// Automatically add the required node edges based on the given ordering.
    void add_render_graph_edges(std::string sub_graph_name, std::vector<std::string> edges);

    /// Add node edge to the specified graph.
    void add_render_graph_edge(std::string sub_graph_name, std::string output_edge, std::string input_edge);
};

} // namespace esr

#endif // ESR_RENDER_APP_H
