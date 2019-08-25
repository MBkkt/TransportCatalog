#pragma once

#include "descriptions.h"
#include "json.h"
#include "svg.h"

#include <map>
#include <string>
#include <unordered_map>
#include <vector>


struct RenderSettings {
    double max_width;
    double max_height;
    double padding;
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
};

class MapRenderer {
 public:
    MapRenderer(const Descriptions::StopsDict &stops_dict,
                const Descriptions::BusesDict &buses_dict,
                const Json::Dict &render_settings_json);

    Svg::Document Render() const;

 private:
    RenderSettings render_settings_;
    const Descriptions::BusesDict &buses_dict_;
    std::map<std::string, Svg::Point> stops_coords_;
    std::unordered_map<std::string, Svg::Color> bus_colors_;

    void RenderBusLines(Svg::Document &svg) const;

    void RenderBusLabels(Svg::Document &svg) const;

    void RenderStopPoints(Svg::Document &svg) const;

    void RenderStopLabels(Svg::Document &svg) const;

    static const std::unordered_map<std::string, void (MapRenderer::*)(Svg::Document &) const> LAYER_ACTIONS;
};
