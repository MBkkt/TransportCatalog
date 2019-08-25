#include "transport_router.h"

using namespace std;


TransportRouter::TransportRouter(const Descriptions::StopsDict &stops_dict,
                                 const Descriptions::BusesDict &buses_dict,
                                 const Json::Dict &routing_settings_json)
    : routing_settings_(MakeRoutingSettings(routing_settings_json)) {
    const size_t vertex_count = stops_dict.size() * 2;
    vertices_info_.resize(vertex_count);
    graph_ = BusGraph(vertex_count);

    FillGraphWithStops(stops_dict);
    FillGraphWithBuses(stops_dict, buses_dict);

    router_ = std::make_unique<Router>(&graph_);
}

TransportRouter::RoutingSettings TransportRouter::MakeRoutingSettings(const Json::Dict &json) {
    return {
        json.at("bus_wait_time").AsInt(),
        json.at("bus_velocity").AsDouble(),
    };
}

void TransportRouter::FillGraphWithStops(const Descriptions::StopsDict &stops_dict) {
    Graph::VertexId vertex_id = 0;

    for (const auto&[stop_name, _] : stops_dict) {
        auto &vertex_ids = stops_vertex_ids_[stop_name];
        vertex_ids.in = vertex_id++;
        vertex_ids.out = vertex_id++;
        vertices_info_[vertex_ids.in] = {stop_name};
        vertices_info_[vertex_ids.out] = {stop_name};

        edges_info_.emplace_back(WaitEdgeInfo{});
        const Graph::EdgeId edge_id = graph_.AddEdge({
                                                         vertex_ids.out,
                                                         vertex_ids.in,
                                                         static_cast<double>(routing_settings_.bus_wait_time)
                                                     });
        assert(edge_id == edges_info_.size() - 1);
    }

    assert(vertex_id == graph_.GetVertexCount());
}

void TransportRouter::FillGraphWithBuses(const Descriptions::StopsDict &stops_dict,
                                         const Descriptions::BusesDict &buses_dict) {
    for (const auto&[_, bus_item] : buses_dict) {
        const auto &bus = *bus_item;
        const size_t stop_count = bus.stops.size();
        if (stop_count <= 1) {
            continue;
        }
        auto compute_distance_from = [&stops_dict, &bus](size_t lhs_idx) {
            return Descriptions::ComputeStopsDistance(*stops_dict.at(bus.stops[lhs_idx]),
                                                      *stops_dict.at(bus.stops[lhs_idx + 1]));
        };
        for (size_t start_stop_idx = 0; start_stop_idx + 1 < stop_count; ++start_stop_idx) {
            const Graph::VertexId start_vertex = stops_vertex_ids_[bus.stops[start_stop_idx]].in;
            int total_distance = 0;
            for (size_t finish_stop_idx = start_stop_idx + 1; finish_stop_idx < stop_count; ++finish_stop_idx) {
                total_distance += compute_distance_from(finish_stop_idx - 1);
                edges_info_.emplace_back(BusEdgeInfo{
                    .bus_name = bus.name,
                    .start_stop_idx = start_stop_idx,
                    .finish_stop_idx = finish_stop_idx,
                });
                const Graph::EdgeId edge_id = graph_.AddEdge({
                                                                 start_vertex,
                                                                 stops_vertex_ids_[bus.stops[finish_stop_idx]].out,
                                                                 total_distance * 1.0 /
                                                                 (routing_settings_.bus_velocity * 1000.0 /
                                                                  60)  // m / (km/h * 1000 / 60) = min
                                                             });
                assert(edge_id == edges_info_.size() - 1);
            }
        }
    }
}

optional<TransportRouter::RouteInfo> TransportRouter::FindRoute(const string &stop_from, const string &stop_to) const {
    const Graph::VertexId vertex_from = stops_vertex_ids_.at(stop_from).out;
    const Graph::VertexId vertex_to = stops_vertex_ids_.at(stop_to).out;
    const auto route = router_->BuildRoute(vertex_from, vertex_to);
    if (!route) {
        return nullopt;
    }

    RouteInfo route_info = {.total_time = route->weight};
    route_info.items.reserve(route->edge_count);
    for (size_t edge_idx = 0; edge_idx < route->edge_count; ++edge_idx) {
        const Graph::EdgeId edge_id = router_->GetRouteEdge(route->id, edge_idx);
        const auto &edge = graph_.GetEdge(edge_id);
        const auto &edge_info = edges_info_[edge_id];
        if (holds_alternative<BusEdgeInfo>(edge_info)) {
            const auto &bus_edge_info = get<BusEdgeInfo>(edge_info);
            route_info.items.emplace_back(RouteInfo::BusItem{
                .bus_name = bus_edge_info.bus_name,
                .time = edge.weight,
                .start_stop_idx = bus_edge_info.start_stop_idx,
                .finish_stop_idx = bus_edge_info.finish_stop_idx,
                .span_count = bus_edge_info.finish_stop_idx - bus_edge_info.start_stop_idx,
            });
        } else {
            const Graph::VertexId vertex_id = edge.from;
            route_info.items.emplace_back(RouteInfo::WaitItem{
                .stop_name = vertices_info_[vertex_id].stop_name,
                .time = edge.weight,
            });
        }
    }

    // Releasing in destructor of some proxy object would be better,
    // but we do not expect exceptions in normal workflow
    router_->ReleaseRoute(route->id);
    return route_info;
}

std::unique_ptr<TransportRouter> TransportRouter::Deserialize(TCProto::TransportRouter *tr) {
    auto db_tr = make_unique<TransportRouter>();
    db_tr->routing_settings_.bus_wait_time = tr->routing_settings().bus_wait_time();
    db_tr->routing_settings_.bus_velocity = tr->routing_settings().bus_velocity();
    auto &graph = db_tr->graph_;
    graph.edges_.reserve(tr->graph().edges_size());
    for (const auto &edge: tr->graph().edges()) {
        graph.edges_.push_back({edge.from(), edge.to(), edge.weight()});
    }
    graph.incidence_lists_.reserve(tr->graph().incidence_lists_size());
    for (const auto &list: tr->graph().incidence_lists()) {
        graph.incidence_lists_.emplace_back(list.edgeids_size());
        for (const auto &edge_id: list.edgeids()) {
            graph.incidence_lists_.back().back() = edge_id;
        }
    }
    db_tr->router_ = make_unique<Router>(&graph, 0);
    auto &rid = db_tr->router_->routes_internal_data_;
    rid.reserve(tr->router().helper_size());
    for (const auto &helper: tr->router().helper()) {
        rid.emplace_back();
        rid.back().reserve(helper.data_size());
        for (const auto &route_data: helper.data()) {
            rid.back().emplace_back();
            if (route_data.type() != TCProto::data_empty) {
                rid.back().back() = Graph::Router<double>::RouteInternalData{
                    route_data.weight(), nullopt
                };
                if (route_data.type() != TCProto::data_half) {
                    rid.back().back().value().prev_edge = route_data.prev_edge();
                }
            }
        }
    }
    for (const auto &item: tr->stops_vertex_id().item()) {
        db_tr->stops_vertex_ids_[item.stop_name()] = StopVertexIds{item.in(), item.out()};
    }
    db_tr->vertices_info_.reserve(tr->vertices_info_stop_name_size());
    for (const auto &stop_name: tr->vertices_info_stop_name()) {
        db_tr->vertices_info_.emplace_back(VertexInfo{stop_name});
    }
    db_tr->edges_info_.reserve(tr->edges_info_size());
    for (const auto &item: tr->edges_info()) {
        if (item.type() == TCProto::WaitInfo) {
            db_tr->edges_info_.emplace_back(WaitEdgeInfo{});
        } else {
            db_tr->edges_info_.emplace_back(
                BusEdgeInfo{item.bus_name(), item.start_stop_idx(), item.finish_stop_idx()});
        }
    }
    return move(db_tr);
}

void TransportRouter::Serialize(TCProto::TransportRouter *tr) const {
    tr->mutable_routing_settings()->set_bus_velocity(routing_settings_.bus_velocity);
    tr->mutable_routing_settings()->set_bus_wait_time(routing_settings_.bus_wait_time);
    for (const auto &edge: graph_.edges_) {
        TCProto::Edge &p_edge = *tr->mutable_graph()->add_edges();
        p_edge.set_from(edge.from);
        p_edge.set_to(edge.to);
        p_edge.set_weight(edge.weight);
    }
    for (const auto &list: graph_.incidence_lists_) {
        TCProto::IncidenceList &p_list = *tr->mutable_graph()->add_incidence_lists();
        for (const auto &edge_id:list) {
            p_list.add_edgeids(edge_id);
        }
    }
    for (const auto &list: router_->routes_internal_data_) {
        TCProto::RouteInternalDataHelper &p_list = *tr->mutable_router()->add_helper();
        for (const auto &route_data: list) {
            TCProto::RouteInternalData &data = *p_list.add_data();
            data.set_type(TCProto::data_empty);
            if (route_data) {
                data.set_type(TCProto::data_half);
                data.set_weight(route_data->weight);
                if (route_data->prev_edge) {
                    data.set_prev_edge(*(route_data->prev_edge));
                    data.set_type(TCProto::data_all);
                }
            }
        }
    }
    for (const auto&[stop_name, idx]: stops_vertex_ids_) {
        auto &item = *tr->mutable_stops_vertex_id()->add_item();
        item.set_stop_name(stop_name);
        item.set_in(idx.in);
        item.set_out(idx.out);
    }
    for (const auto &info: vertices_info_) {
        tr->add_vertices_info_stop_name(info.stop_name);
    }
    for (const auto &item: edges_info_) {
        TCProto::EdgeInfo &edges_info = *tr->add_edges_info();
        if (std::holds_alternative<BusEdgeInfo>(item)) {
            edges_info.set_type(TCProto::BusInfo);
            const auto &bus_item = get<BusEdgeInfo>(item);
            edges_info.set_bus_name(bus_item.bus_name);
            edges_info.set_start_stop_idx(bus_item.start_stop_idx);
            edges_info.set_finish_stop_idx(bus_item.finish_stop_idx);
        } else {
            edges_info.set_type(TCProto::WaitInfo);
        }
    }
}
