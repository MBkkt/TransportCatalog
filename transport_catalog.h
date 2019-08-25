#pragma once

#include "descriptions.h"
#include "json.h"
#include "transport_router.h"
#include "utils.h"
#include "svg.h"

#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace Responses {
struct Stop {
    std::set<std::string> bus_names;
    Svg::Point center;
};

struct Bus {
    size_t stop_count = 0;
    size_t unique_stop_count = 0;
    int road_route_length = 0;
    double geo_route_length = 0.0;
    Svg::Polyline line;
};
}

class TransportCatalog {
 private:
    using Bus = Responses::Bus;
    using Stop = Responses::Stop;

 public:
    TransportCatalog(std::vector<Descriptions::InputQuery> data,
                     const Json::Dict &routing_settings_json,
                     const Json::Dict &render_settings_json);

    const Stop *GetStop(const std::string &name) const;

    const Bus *GetBus(const std::string &name) const;

    std::optional<TransportRouter::RouteInfo> FindRoute(const std::string &stop_from, const std::string &stop_to) const;

    std::string RenderMap() const;

 private:
    struct RenderSettings {
        double width;
        double height;
        double padding;
        double stop_radius;
        double line_width;
        double underlayer_width;
        Svg::Point stop_label_offset;
        std::vector<Svg::Color> color_palette;
        Svg::Color underlayer_color;
        uint32_t stop_label_font_size;

        explicit RenderSettings(const Json::Dict &json);
    };

    static int ComputeRoadRouteLength(
        const std::vector<std::string> &stops,
        const Descriptions::StopsDict &stops_dict
    );

    static double ComputeGeoRouteDistance(
        const std::vector<std::string> &stops,
        const Descriptions::StopsDict &stops_dict
    );

    std::unordered_map<std::string, Stop> stops_;
    std::unordered_map<std::string, Bus> buses_;
    std::unique_ptr<TransportRouter> router_;
    RenderSettings render_settings_;
    mutable std::string cache_;
};
