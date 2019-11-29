#pragma once

#include "descriptions.h"
#include "json.h"
#include "svg.h"
#include "transport_router.h"

#include "map_renderer.pb.h"

#include <map>
#include <string>
#include <unordered_map>
#include <vector>


struct RenderSettings {
    double max_width;
    double max_height;
    double padding;
    double outer_margin;
    std::vector<Svg::Color> palette;
    double line_width;
    Svg::Color underlayer_color;
    double underlayer_width;
    double stop_radius;
    Svg::Point bus_label_offset;
    int bus_label_font_size;
    Svg::Point stop_label_offset;
    int stop_label_font_size;
    std::vector<std::string> layers;

    void Serialize(TCProto::RenderSettings &proto) const;

    static RenderSettings Deserialize(const TCProto::RenderSettings &proto);
};

class MapRenderer {
 public:
    MapRenderer(const Descriptions::StopsDict &stops_dict,
                const Descriptions::BusesDict &buses_dict,
                const Json::Dict &render_settings_json);

    void Serialize(TCProto::MapRenderer &proto);

    static std::unique_ptr<MapRenderer> Deserialize(const TCProto::MapRenderer &proto);

    const Svg::Document& Render() const;

    Svg::Document RenderRoute(Svg::Document whole_map, const TransportRouter::RouteInfo &route) const;

    MapRenderer() = default;

 private:
    RenderSettings render_settings_;
    std::map<std::string, Svg::Point> stops_coords_;
    std::unordered_map<std::string, Svg::Color> bus_colors_;
    // TODO: move instead of copy
    std::map<std::string, Descriptions::Bus> buses_dict_;
    mutable std::optional<Svg::Document> whole_map_;

    void RenderBusLabel(Svg::Document &svg, const std::string &bus_name, const std::string &stop_name) const;

    void RenderStopPoint(Svg::Document &svg, Svg::Point point) const;

    void RenderStopLabel(Svg::Document &svg, Svg::Point point, const std::string &name) const;

    void RenderBusLines(Svg::Document &svg) const;

    void RenderBusLabels(Svg::Document &svg) const;

    void RenderStopPoints(Svg::Document &svg) const;

    void RenderStopLabels(Svg::Document &svg) const;

    void RenderRouteBusLines(Svg::Document &svg, const TransportRouter::RouteInfo &route) const;

    void RenderRouteBusLabels(Svg::Document &svg, const TransportRouter::RouteInfo &route) const;

    void RenderRouteStopPoints(Svg::Document &svg, const TransportRouter::RouteInfo &route) const;

    void RenderRouteStopLabels(Svg::Document &svg, const TransportRouter::RouteInfo &route) const;

    static const std::unordered_map<
        std::string,
        void (MapRenderer::*)(Svg::Document &) const
    > MAP_LAYER_ACTIONS;

    static const std::unordered_map<
        std::string,
        void (MapRenderer::*)(Svg::Document &, const TransportRouter::RouteInfo &) const
    > ROUTE_LAYER_ACTIONS;
};
