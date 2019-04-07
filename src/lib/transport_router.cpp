#include "transport_router.h"


using namespace std;

TransportRouter::TransportRouter(Descriptions::StopsDict const & stops_dict,
                                 Descriptions::BusesDict const & buses_dict,
                                 Json::Dict const & routing_settings_json)
    : routing_settings_(makeRoutingSettings(routing_settings_json)) {
    const size_t vertex_count = stops_dict.size() * 2;
    vertices_info_.resize(vertex_count);
    graph_ = BusGraph(vertex_count);

    fillGraphWithStops(stops_dict);
    fillGraphWithBuses(stops_dict, buses_dict);

    router_ = std::make_unique<Router>(graph_);
}

TransportRouter::RoutingSettings TransportRouter::makeRoutingSettings(Json::Dict const & json) {
    return {json.at("bus_wait_time").asInt(), json.at("bus_velocity").asDouble(),};
}

void TransportRouter::fillGraphWithStops(Descriptions::StopsDict const & stops_dict) {
    Graph::VertexId vertex_id = 0;

    for (const auto&[stop_name, _] : stops_dict) {
        auto & vertex_ids = stops_vertex_ids_[stop_name];
        vertex_ids.in = vertex_id++;
        vertex_ids.out = vertex_id++;
        vertices_info_[vertex_ids.in] = {stop_name};
        vertices_info_[vertex_ids.out] = {stop_name};

        edges_info_.emplace_back(WaitEdgeInfo {});
        Graph::EdgeId const edge_id = graph_.addEdge(
            {
                vertex_ids.out,
                vertex_ids.in,
                static_cast<double>(routing_settings_.bus_wait_time)
            }
        );
        assert(edge_id == edges_info_.size() - 1);
    }

    assert(vertex_id == graph_.getVertexCount());
}

void TransportRouter::fillGraphWithBuses(Descriptions::StopsDict const & stops_dict,
                                         Descriptions::BusesDict const & buses_dict) {
    for (auto const &[_, bus_item] : buses_dict) {
        auto const & bus = *bus_item;
        size_t const stop_count = bus.stops.size();

        if (stop_count <= 1) {
            continue;
        }

        auto compute_distance_from = [&stops_dict, &bus](size_t lhs_idx) {
            return Descriptions::computeStopsDistance(*stops_dict.at(bus.stops[lhs_idx]),
                                                      *stops_dict.at(bus.stops[lhs_idx + 1]));
        };

        for (size_t start_stop_idx = 0; start_stop_idx + 1 < stop_count; ++start_stop_idx) {
            Graph::VertexId const start_vertex = stops_vertex_ids_[bus.stops[start_stop_idx]].in;
            int total_distance = 0;

            for (size_t finish_stop_idx = start_stop_idx + 1;
                 finish_stop_idx < stop_count; ++finish_stop_idx) {
                total_distance += compute_distance_from(finish_stop_idx - 1);
                edges_info_.emplace_back(BusEdgeInfo {
                    .bus_name = bus.name,
                    .span_count = finish_stop_idx - start_stop_idx,
                });
                Graph::EdgeId const edge_id = graph_.addEdge(
                    {
                        start_vertex,
                        stops_vertex_ids_[bus.stops[finish_stop_idx]].out,
                        total_distance * 1. / (
                            routing_settings_.bus_velocity * 1000. / 60.
                        )  // m / (km/h * 1000 / 60) = min
                    }
                );
                assert(edge_id == edges_info_.size() - 1);
            }
        }
    }
}

optional<TransportRouter::RouteInfo>
TransportRouter::findRoute(string const & stop_from, string const & stop_to) const {
    Graph::VertexId const vertex_from = stops_vertex_ids_.at(stop_from).out;
    Graph::VertexId const vertex_to = stops_vertex_ids_.at(stop_to).out;
    auto const route = router_->buildRoute(vertex_from, vertex_to);

    if (!route) {
        return nullopt;
    }

    RouteInfo route_info = {.total_time = route->weight};
    route_info.items.reserve(route->edge_count);

    for (size_t edge_idx = 0; edge_idx < route->edge_count; ++edge_idx) {
        Graph::EdgeId const edge_id = router_->getRouteEdge(route->id, edge_idx);
        auto const & edge = graph_.getEdge(edge_id);
        auto const & edge_info = edges_info_[edge_id];

        if (holds_alternative<BusEdgeInfo>(edge_info)) {
            auto const & bus_edge_info = get<BusEdgeInfo>(edge_info);
            route_info.items.emplace_back(RouteInfo::BusItem {
                .bus_name = bus_edge_info.bus_name,
                .time = edge.weight,
                .span_count = bus_edge_info.span_count,
            });
        } else {
            const Graph::VertexId vertex_id = edge.from;
            route_info.items.emplace_back(RouteInfo::WaitItem {
                .stop_name = vertices_info_[vertex_id].stop_name,
                .time = edge.weight,
            });
        }
    }

    router_->releaseRoute(route->id);
    return route_info;
}
