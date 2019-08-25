#pragma once

#include "sphere.h"
#include "svg.h"
#include "utils.h"
#include "descriptions.h"

#include <algorithm>
#include <cmath>
#include <vector>
#include <optional>


namespace Sphere {
struct Stop {
    std::string name;
    Point position;
    std::unordered_set<std::string> neighbours;
};

class Projector {
 public:
    template<typename StopContainer>
    Projector(StopContainer stops,
              double max_width, double max_height, double padding);

    Svg::Point operator()(Point point) const;

 private:
    const double padding_;
    double x_step = 0.0;
    double y_step = 0.0;
    double max_height_;
    std::map<double, int> lon_points_;
    std::map<double, int> lat_points_;
};

template<typename StopContainer>
Projector::Projector(StopContainer stops, double max_width, double max_height, double padding)
    : padding_(padding), max_height_(max_height) {
    if (stops.empty()) {
        return;
    }

    std::sort(stops.begin(), stops.end(),
              [](const auto &l, const auto &r) { return l.position.longitude < r.position.longitude; });

    size_t id = 0;
    lon_points_[stops[0].position.longitude] = id;
    for (size_t i = 1, j = 0; i < stops.size(); ++i) {
        lon_points_[stops[i].position.longitude] = id;
        if (stops[j].position.longitude == stops[i].position.longitude) {
            continue;
        }
        for (size_t k = j; k < i; ++k) {
            if (stops[i].neighbours.count(stops[k].name) ||
                stops[k].neighbours.count(stops[i].name)) {
                lon_points_[stops[i].position.longitude] = ++id;
                j = i;
                break;
            }
        }
    }

    if (id >= 1) {
        x_step = (max_width - 2 * padding) / id;
    }


    std::sort(stops.begin(), stops.end(),
              [](const auto &l, const auto &r) { return l.position.latitude < r.position.latitude; });
    id = 0;
    lat_points_[stops[0].position.latitude] = id;
    for (size_t i = 1, j = 0; i < stops.size(); ++i) {
        lat_points_[stops[i].position.latitude] = id;
        if (stops[j].position.latitude == stops[i].position.latitude) {
            continue;
        }
        for (size_t k = j; k < i; ++k) {
            if (stops[i].neighbours.count(stops[k].name) ||
                stops[k].neighbours.count(stops[i].name)) {
                lat_points_[stops[i].position.latitude] = ++id;
                j = i;
                break;
            }
        }
    }
    if (id >= 1) {
        y_step = (max_height - 2 * padding) / id;
    }

}

}
