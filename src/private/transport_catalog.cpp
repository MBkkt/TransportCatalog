#include "transport_catalog.h"
#include "utils.h"

#include "transport_catalog.pb.h"

#include <algorithm>
#include <iterator>
#include <map>
#include <optional>
#include <sstream>
#include <unordered_map>

using namespace std;

TransportCatalog::TransportCatalog(
    vector<Descriptions::InputQuery> data,
    const Json::Dict &routing_settings_json,
    const Json::Dict &render_settings_json
) {
    auto stops_end = partition(begin(data), end(data), [](const auto &item) {
        return holds_alternative<Descriptions::Stop>(item);
    });

    Descriptions::StopsDict stops_dict;
    for (const auto &item : Range{begin(data), stops_end}) {
        const auto &stop = get<Descriptions::Stop>(item);
        stops_dict[stop.name] = &stop;
        stops_.insert({stop.name, {}});
    }

    Descriptions::BusesDict buses_dict;
    for (const auto &item : Range{stops_end, end(data)}) {
        const auto &bus = get<Descriptions::Bus>(item);

        buses_dict[bus.name] = &bus;
        buses_[bus.name] = Bus{
            bus.stops.size(),
            ComputeUniqueItemsCount(AsRange(bus.stops)),
            ComputeRoadRouteLength(bus.stops, stops_dict),
            ComputeGeoRouteDistance(bus.stops, stops_dict)
        };

        for (const string &stop_name : bus.stops) {
            stops_.at(stop_name).bus_names.insert(bus.name);
        }
    }

    router_ = make_unique<TransportRouter>(stops_dict, buses_dict, routing_settings_json);

    map_renderer_ = make_unique<MapRenderer>(stops_dict, buses_dict, render_settings_json);
}

const TransportCatalog::Stop *TransportCatalog::GetStop(const string &name) const {
    return GetValuePointer(stops_, name);
}

const TransportCatalog::Bus *TransportCatalog::GetBus(const string &name) const {
    return GetValuePointer(buses_, name);
}

optional<TransportRouter::RouteInfo> TransportCatalog::FindRoute(const string &stop_from, const string &stop_to) const {
    return router_->FindRoute(stop_from, stop_to);
}

string TransportCatalog::RenderMap() const {
    ostringstream out;
    map_renderer_->Render().Render(out);
    return out.str();
}

string TransportCatalog::RenderRoute(const TransportRouter::RouteInfo &route) const {
    ostringstream out;
    BuildRouteMap(route).Render(out);
    return out.str();
}

size_t TransportCatalog::ComputeRoadRouteLength(
    const vector<string> &stops,
    const Descriptions::StopsDict &stops_dict
) {
    size_t result = 0;
    for (size_t i = 1; i < stops.size(); ++i) {
        result += Descriptions::ComputeStopsDistance(*stops_dict.at(stops[i - 1]), *stops_dict.at(stops[i]));
    }
    return result;
}

double TransportCatalog::ComputeGeoRouteDistance(
    const vector<string> &stops,
    const Descriptions::StopsDict &stops_dict
) {
    double result = 0;
    for (size_t i = 1; i < stops.size(); ++i) {
        result += Sphere::Distance(
            stops_dict.at(stops[i - 1])->position, stops_dict.at(stops[i])->position
        );
    }
    return result;
}

Svg::Document TransportCatalog::BuildRouteMap(const TransportRouter::RouteInfo &route) const {
    return map_renderer_->RenderRoute(map_renderer_->Render(), route);
}

string TransportCatalog::Serialize() const {
    TCProto::TransportCatalog db_proto;

    for (const auto&[name, stop] : stops_) {
        TCProto::StopResponse &stop_proto = *db_proto.add_stops();
        stop_proto.set_name(name);
        for (const string &bus_name : stop.bus_names) {
            stop_proto.add_bus_names(bus_name);
        }
    }

    for (const auto&[name, bus] : buses_) {
        TCProto::BusResponse &bus_proto = *db_proto.add_buses();
        bus_proto.set_name(name);
        bus_proto.set_stop_count(bus.stop_count);
        bus_proto.set_unique_stop_count(bus.unique_stop_count);
        bus_proto.set_road_route_length(bus.road_route_length);
        bus_proto.set_geo_route_length(bus.geo_route_length);
    }

    router_->Serialize(*db_proto.mutable_router());
    map_renderer_->Serialize(*db_proto.mutable_renderer());

    return db_proto.SerializeAsString();
}

TransportCatalog TransportCatalog::Deserialize(const string &data) {
    TCProto::TransportCatalog proto;
    assert(proto.ParseFromString(data));

    TransportCatalog catalog;

    for (const TCProto::StopResponse &stop_proto : proto.stops()) {
        Stop &stop = catalog.stops_[stop_proto.name()];
        for (const string &bus_name : stop_proto.bus_names()) {
            stop.bus_names.insert(bus_name);
        }
    }

    for (const TCProto::BusResponse &bus_proto : proto.buses()) {
        Bus &bus = catalog.buses_[bus_proto.name()];
        bus.stop_count = bus_proto.stop_count();
        bus.unique_stop_count = bus_proto.unique_stop_count();
        bus.road_route_length = bus_proto.road_route_length();
        bus.geo_route_length = bus_proto.geo_route_length();
    }

    catalog.router_ = TransportRouter::Deserialize(proto.router());
    catalog.map_renderer_ = MapRenderer::Deserialize(proto.renderer());

    return catalog;
}
