#ifndef __ROUTER_H__
#define __ROUTER_H__
#pragma once

#include "graph.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iterator>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Graph {

    template<typename Weight>
    class Router {
    private:
        using Graph = DirectedWeightedGraph<Weight>;

    public:
        Router(Graph const & graph);

        using RouteId = uint64_t;

        struct RouteInfo {
            RouteId id;
            Weight weight;
            size_t edge_count;
        };

        std::optional<RouteInfo> buildRoute(VertexId from, VertexId to) const;

        EdgeId getRouteEdge(RouteId route_id, size_t edge_idx) const;

        void releaseRoute(RouteId route_id);

    private:
        Graph const & graph_;

        struct RouteInternalData {
            Weight weight;
            std::optional<EdgeId> prev_edge;
        };
        using RoutesInternalData = std::vector<std::vector<std::optional<RouteInternalData>>>;

        using ExpandedRoute = std::vector<EdgeId>;
        RouteId mutable next_route_id_ = 0;

        std::unordered_map<RouteId, ExpandedRoute> mutable expanded_routes_cache_;

        void initializeRoutesInternalData(Graph const & graph) {
            const size_t vertex_count = graph.getVertexCount();
            for (VertexId vertex = 0; vertex < vertex_count; ++vertex) {
                routes_internal_data_[vertex][vertex] = RouteInternalData {0, std::nullopt};
                for (EdgeId const edge_id : graph.getIncidentEdges(vertex)) {
                    auto const & edge = graph.getEdge(edge_id);
                    assert(edge.weight >= 0);
                    auto & route_internal_data = routes_internal_data_[vertex][edge.to];
                    if (!route_internal_data || route_internal_data->weight > edge.weight) {
                        route_internal_data = RouteInternalData {edge.weight, edge_id};
                    }
                }
            }
        }

        void relaxRoute(VertexId vertex_from, VertexId vertex_to,
                        RouteInternalData const & route_from, RouteInternalData const & route_to) {
            auto & route_relaxing = routes_internal_data_[vertex_from][vertex_to];
            Weight const candidate_weight = route_from.weight + route_to.weight;
            if (!route_relaxing || candidate_weight < route_relaxing->weight) {
                route_relaxing = {
                    candidate_weight,
                    route_to.prev_edge ? route_to.prev_edge : route_from.prev_edge
                };
            }
        }

        void relaxRoutesInternalDataThroughVertex(size_t vertex_count, VertexId vertex_through) {
            for (VertexId vertex_from = 0; vertex_from < vertex_count; ++vertex_from) {
                if (auto const & route_from = routes_internal_data_[vertex_from][vertex_through]) {
                    for (VertexId vertex_to = 0; vertex_to < vertex_count; ++vertex_to) {
                        if (auto const & route_to = routes_internal_data_[vertex_through][vertex_to]) {
                            relaxRoute(vertex_from, vertex_to, *route_from, *route_to);
                        }
                    }
                }
            }
        }

        RoutesInternalData routes_internal_data_;
    };


    template<typename Weight>
    Router<Weight>::Router(Graph const & graph)
        : graph_(graph),
          routes_internal_data_(
              graph.getVertexCount(),
              std::vector<std::optional<RouteInternalData>>(graph.getVertexCount())) {
        initializeRoutesInternalData(graph);

        size_t const vertex_count = graph.getVertexCount();

        for (VertexId vertex_through = 0; vertex_through < vertex_count; ++vertex_through) {
            relaxRoutesInternalDataThroughVertex(vertex_count, vertex_through);
        }
    }

    template<typename Weight>
    std::optional<typename Router<Weight>::RouteInfo> Router<Weight>::buildRoute(VertexId from, VertexId to) const {
        auto const & route_internal_data = routes_internal_data_[from][to];
        if (!route_internal_data) {
            return std::nullopt;
        }
        Weight const weight = route_internal_data->weight;

        std::vector<EdgeId> edges;

        for (std::optional<EdgeId> edge_id = route_internal_data->prev_edge;
             edge_id;
             edge_id = routes_internal_data_[from][graph_.getEdge(*edge_id).from]->prev_edge) {
            edges.push_back(*edge_id);
        }
        std::reverse(std::begin(edges), std::end(edges));

        RouteId const route_id = next_route_id_++;
        size_t const route_edge_count = edges.size();
        expanded_routes_cache_[route_id] = std::move(edges);
        return RouteInfo {route_id, weight, route_edge_count};
    }

    template<typename Weight>
    EdgeId Router<Weight>::getRouteEdge(RouteId route_id, size_t edge_idx) const {
        return expanded_routes_cache_.at(route_id)[edge_idx];
    }

    template<typename Weight>
    void Router<Weight>::releaseRoute(RouteId route_id) {
        expanded_routes_cache_.erase(route_id);
    }

}

#endif /* __ROUTER_H__ */
