#ifndef __TRANSPORT_ROUTER_H__
#define __TRANSPORT_ROUTER_H__
#pragma once

#include "descriptions.h"
#include "graph.h"
#include "json.h"
#include "router.h"

#include <memory>
#include <unordered_map>
#include <vector>


class TransportRouter {
private:
    using BusGraph = Graph::DirectedWeightedGraph<double>;
    using Router = Graph::Router<double>;

public:
    TransportRouter(Descriptions::StopsDict const & stops_dict,
                    Descriptions::BusesDict const & buses_dict,
                    Json::Dict const & routing_settings_json);

    struct RouteInfo {
        double total_time;

        struct BusItem {
            std::string bus_name;
            double time;
            size_t span_count;
        };
        struct WaitItem {
            std::string stop_name;
            double time;
        };

        using Item = std::variant<BusItem, WaitItem>;
        std::vector<Item> items;
    };

    std::optional<RouteInfo> findRoute(std::string const & stop_from, std::string const & stop_to) const;

private:
    struct RoutingSettings {
        int bus_wait_time;  // in minutes
        double bus_velocity;  // km/h
    };

    static RoutingSettings makeRoutingSettings(Json::Dict const & json);

    void fillGraphWithStops(Descriptions::StopsDict const & stops_dict);

    void fillGraphWithBuses(Descriptions::StopsDict const & stops_dict,
                            Descriptions::BusesDict const & buses_dict);

    struct StopVertexIds {
        Graph::VertexId in;
        Graph::VertexId out;
    };
    struct VertexInfo {
        std::string stop_name;
    };

    struct BusEdgeInfo {
        std::string bus_name;
        size_t span_count;
    };
    struct WaitEdgeInfo {
    };
    using EdgeInfo = std::variant<BusEdgeInfo, WaitEdgeInfo>;

    std::unordered_map<std::string, StopVertexIds> stops_vertex_ids_;
    RoutingSettings routing_settings_;
    BusGraph graph_;
    std::unique_ptr<Router> router_;
    std::vector<VertexInfo> vertices_info_;
    std::vector<EdgeInfo> edges_info_;
};

#endif /* __TRANSPORT_ROUTER_H__ */
