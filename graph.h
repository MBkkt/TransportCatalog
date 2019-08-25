#pragma once

#include "utils.h"
#include "graph.pb.h"

#include <cstdlib>
#include <deque>
#include <type_traits>
#include <vector>

namespace Graph {

using VertexId = size_t;
using EdgeId = size_t;

template<typename Weight>
struct Edge {
    VertexId from;
    VertexId to;
    Weight weight;
};

template<typename Weight>
class DirectedWeightedGraph {
 private:
    using IncidenceList = std::vector<EdgeId>;
    using IncidentEdgesRange = Range<typename IncidenceList::const_iterator>;

 public:
    DirectedWeightedGraph(size_t vertex_count = 0);

    EdgeId AddEdge(const Edge<Weight> &edge);

    size_t GetVertexCount() const;

    size_t GetEdgeCount() const;

    const Edge<Weight> &GetEdge(EdgeId edge_id) const;

    IncidentEdgesRange GetIncidentEdges(VertexId vertex) const;

    void Serialize(GraphProto::DirectedWeightedGraph &proto) const;

    static DirectedWeightedGraph Deserialize(const GraphProto::DirectedWeightedGraph &proto);

 private:
    std::vector<Edge<Weight>> edges_;
    std::vector<IncidenceList> incidence_lists_;
};


template<typename Weight>
DirectedWeightedGraph<Weight>::DirectedWeightedGraph(size_t vertex_count) : incidence_lists_(vertex_count) {}

template<typename Weight>
EdgeId DirectedWeightedGraph<Weight>::AddEdge(const Edge<Weight> &edge) {
    edges_.push_back(edge);
    const EdgeId id = edges_.size() - 1;
    incidence_lists_[edge.from].push_back(id);
    return id;
}

template<typename Weight>
size_t DirectedWeightedGraph<Weight>::GetVertexCount() const {
    return incidence_lists_.size();
}

template<typename Weight>
size_t DirectedWeightedGraph<Weight>::GetEdgeCount() const {
    return edges_.size();
}

template<typename Weight>
const Edge<Weight> &DirectedWeightedGraph<Weight>::GetEdge(EdgeId edge_id) const {
    return edges_[edge_id];
}

template<typename Weight>
typename DirectedWeightedGraph<Weight>::IncidentEdgesRange
DirectedWeightedGraph<Weight>::GetIncidentEdges(VertexId vertex) const {
    const auto &edges = incidence_lists_[vertex];
    return {std::begin(edges), std::end(edges)};
}

template<typename Weight>
void DirectedWeightedGraph<Weight>::Serialize(GraphProto::DirectedWeightedGraph &proto) const {
    static_assert(std::is_same_v<Weight, double>, "Serialization is implemented only for double weights");

    for (const auto &edge : edges_) {
        auto &edge_proto = *proto.add_edges();
        edge_proto.set_from(edge.from);
        edge_proto.set_to(edge.to);
        edge_proto.set_weight(edge.weight);
    }

    for (const auto &incidence_list : incidence_lists_) {
        auto &incidence_list_proto = *proto.add_incidence_lists();
        for (const auto edge_id : incidence_list) {
            incidence_list_proto.add_edge_ids(edge_id);
        }
    }
}

template<typename Weight>
DirectedWeightedGraph<Weight>
DirectedWeightedGraph<Weight>::Deserialize(const GraphProto::DirectedWeightedGraph &proto) {
    static_assert(std::is_same_v<Weight, double>, "Serialization is implemented only for double weights");

    DirectedWeightedGraph graph;

    graph.edges_.reserve(proto.edges_size());
    for (const auto &edge_proto : proto.edges()) {
        auto &edge = graph.edges_.emplace_back();
        edge.from = edge_proto.from();
        edge.to = edge_proto.to();
        edge.weight = edge_proto.weight();
    }

    graph.incidence_lists_.reserve(proto.incidence_lists_size());
    for (const auto &incidence_list_proto : proto.incidence_lists()) {
        auto &incidence_list = graph.incidence_lists_.emplace_back();
        incidence_list.reserve(incidence_list_proto.edge_ids_size());
        for (const auto edge_id : incidence_list_proto.edge_ids()) {
            incidence_list.push_back(edge_id);
        }
    }

    return graph;
}
}
