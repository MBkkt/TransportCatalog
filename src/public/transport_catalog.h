#pragma once

#include "descriptions.h"
#include "json.h"
#include "map_renderer.h"
#include "svg.h"
#include "transport_router.h"
#include "utils.h"

#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace Responses {
struct Stop {
    std::set<std::string> bus_names;
};

struct Bus {
    size_t stop_count = 0;
    size_t unique_stop_count = 0;
    size_t road_route_length = 0;
    double geo_route_length = 0.0;
};
}

class TransportCatalog {
 private:
    using Bus = Responses::Bus;
    using Stop = Responses::Stop;

 public:
    TransportCatalog(
        std::vector<Descriptions::InputQuery> data,
        const Json::Dict &routing_settings_json,
        const Json::Dict &render_settings_json
    );

    const Stop *GetStop(const std::string &name) const;

    const Bus *GetBus(const std::string &name) const;

    std::optional<TransportRouter::RouteInfo> FindRoute(const std::string &stop_from, const std::string &stop_to) const;

    std::string RenderMap() const;

    std::string RenderRoute(const TransportRouter::RouteInfo &route) const;

    std::string Serialize() const;

    static TransportCatalog Deserialize(const std::string &data);

 private:
    TransportCatalog() = default;

    static size_t ComputeRoadRouteLength(
        const std::vector<std::string> &stops,
        const Descriptions::StopsDict &stops_dict
    );

    static double ComputeGeoRouteDistance(
        const std::vector<std::string> &stops,
        const Descriptions::StopsDict &stops_dict
    );

    Svg::Document BuildRouteMap(const TransportRouter::RouteInfo &route) const;

    std::unordered_map<std::string, Stop> stops_;
    std::unordered_map<std::string, Bus> buses_;
    std::unique_ptr<TransportRouter> router_;
    std::unique_ptr<MapRenderer> map_renderer_;
    Svg::Document map_;
};
