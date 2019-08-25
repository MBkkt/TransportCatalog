#pragma GCC optimize("Ofast")

#include "transport_catalog.h"
#include "svg.h"

#include <algorithm>
#include <vector>
#include <sstream>

using namespace std;

TransportCatalog::TransportCatalog(vector<Descriptions::InputQuery> data,
                                   const Json::Dict &routing_settings_json,
                                   const Json::Dict &render_settings_json)
    : render_settings_(render_settings_json) {
    auto stops_end = partition(begin(data), end(data), [](const auto &item) {
        return holds_alternative<Descriptions::Stop>(item);
    });

    double min_lat = numeric_limits<double>::max();
    double min_lon = numeric_limits<double>::max();
    double max_lat = numeric_limits<double>::min();
    double max_lon = numeric_limits<double>::min();

    Descriptions::StopsDict stops_dict;
    for (const auto &item : Range{begin(data), stops_end}) {
        const auto &stop = get<Descriptions::Stop>(item);
        stops_dict[stop.name] = &stop;
        stops_.insert({stop.name, {{}, {stop.position.longitude, stop.position.latitude}}});

        min_lat = min(min_lat, stop.position.latitude);
        min_lon = min(min_lon, stop.position.longitude);
        max_lat = max(max_lat, stop.position.latitude);
        max_lon = max(max_lon, stop.position.longitude);
    }

    double width_zoom_coef = numeric_limits<double>::max();
    double height_zoom_coef;
    double zoom_coef = 0.0;
    if (max_lon - min_lon != 0) {
        width_zoom_coef = (render_settings_.width - 2 * render_settings_.padding) / (max_lon - min_lon);
        zoom_coef = width_zoom_coef;
    }
    if (max_lat - min_lat != 0) {
        height_zoom_coef = (render_settings_.height - 2 * render_settings_.padding) / (max_lat - min_lat);
        zoom_coef = min(width_zoom_coef, height_zoom_coef);
    }

    for (auto&[stop_name, stop]: stops_) {
        stop.center = {
            (stop.center.x - min_lon) * zoom_coef + render_settings_.padding,
            (max_lat - stop.center.y) * zoom_coef + render_settings_.padding
        };
    }


    Descriptions::BusesDict buses_dict;
    for (const auto &item : Range{stops_end, end(data)}) {
        Svg::Polyline line;
        line.SetStrokeWidth(render_settings_.line_width)
            .SetStrokeLineCap("round")
            .SetStrokeLineJoin("round");
        const auto &bus = get<Descriptions::Bus>(item);
        buses_dict[bus.name] = &bus;

        for (const string &stop_name : bus.stops) {
            line.AddPoint(stops_.at(stop_name).center);
            stops_.at(stop_name).bus_names.insert(bus.name);
        }

        buses_[bus.name] = Bus{
            bus.stops.size(),
            ComputeUniqueItemsCount(AsRange(bus.stops)),
            ComputeRoadRouteLength(bus.stops, stops_dict),
            ComputeGeoRouteDistance(bus.stops, stops_dict),
            line
        };

    }

    router_ = make_unique<TransportRouter>(stops_dict, buses_dict, routing_settings_json);
}

namespace {

Svg::Color GetColor(const Json::Node &value) {
    if (value.IsString()) {
        return value.AsString();
    }
    const auto &node = value.AsArray();
    if (node.size() == 3) {
        return Svg::Rgba{node[0].As<uint8_t>(), node[1].As<uint8_t>(), node[2].As<uint8_t>()};
    } else {
        return Svg::Rgba{node[0].As<uint8_t>(), node[1].As<uint8_t>(), node[2].As<uint8_t>(),
                         node[3].AsDouble()};
    }
}

}

TransportCatalog::RenderSettings::RenderSettings(const Json::Dict &json) {
    width = json.at("width").AsDouble();
    height = json.at("height").AsDouble();
    padding = json.at("padding").AsDouble();
    stop_radius = json.at("stop_radius").AsDouble();
    line_width = json.at("line_width").AsDouble();
    underlayer_width = json.at("underlayer_width").AsDouble();
    stop_label_font_size = json.at("stop_label_font_size").AsInt();
    const auto &array = json.at("stop_label_offset").AsArray();
    stop_label_offset = {array[0].AsDouble(), array[1].AsDouble()};
    underlayer_color = GetColor(json.at("underlayer_color"));
    color_palette.reserve(100);
    for (const auto &json_color: json.at("color_palette").AsArray()) {
        color_palette.push_back(move(GetColor(json_color)));
    }
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

int TransportCatalog::ComputeRoadRouteLength(
    const vector<string> &stops,
    const Descriptions::StopsDict &stops_dict
) {
    int result = 0;
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

string TransportCatalog::RenderMap() const {
    if (!cache_.empty()) {
        return cache_;
    }
    Svg::Document document{};

    vector<pair<std::string, Svg::Polyline>> sort_buses;
    sort_buses.reserve(buses_.size());
    for (auto&[bus_name, bus]: buses_) {
        sort_buses.emplace_back(bus_name, bus.line);
    }
    sort(begin(sort_buses), end(sort_buses));
    size_t i = 0;
    for (auto &[_, line]: sort_buses) {
        document.Add(line.SetStrokeColor(render_settings_.color_palette[i]));
        i = (i + 1) % render_settings_.color_palette.size();
    }
    vector<pair<string, Svg::Point>> sort_stops;
    sort_stops.reserve(stops_.size());
    for (auto&[stop_name, stop]: stops_) {
        sort_stops.emplace_back(stop_name, stop.center);
    }
    sort(begin(sort_stops), end(sort_stops));

    Svg::Circle circle;
    circle.SetRadius(render_settings_.stop_radius)
        .SetFillColor("white");
    for (auto&[_, center]: sort_stops) {
        document.Add(circle.SetCenter(center));
    }

    Svg::Text text;
    text.SetOffset(render_settings_.stop_label_offset)
        .SetFontSize(render_settings_.stop_label_font_size)
        .SetFontFamily("Verdana");
    Svg::Text underlayer = text;
    text.SetFillColor("black");
    underlayer.SetFillColor(render_settings_.underlayer_color)
        .SetStrokeColor(render_settings_.underlayer_color)
        .SetStrokeWidth(render_settings_.underlayer_width)
        .SetStrokeLineJoin("round").SetStrokeLineCap("round");
    for (auto&[stop_name, center]: sort_stops) {
        document.Add(underlayer.SetPoint(center).SetData(stop_name));
        document.Add(text.SetPoint(center).SetData(stop_name));
    }
    ostringstream helper;
    document.Render(helper);
    return cache_ = helper.str();
}


