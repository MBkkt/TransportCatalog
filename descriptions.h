#pragma once

#include "json.h"
#include "sphere.h"

#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>


namespace Descriptions {
struct Stop {
    std::string name;
    Sphere::Point position;
    std::unordered_map<std::string, size_t> distances;

    static Stop ParseFrom(const Json::Dict &attrs);
};

size_t ComputeStopsDistance(const Stop &lhs, const Stop &rhs);

struct Bus {
    std::string name;
    std::vector<std::string> stops;
    std::vector<std::string> endpoints;

    static Bus ParseFrom(const Json::Dict &attrs);
};

using InputQuery = std::variant<Stop, Bus>;

std::vector<InputQuery> ReadDescriptions(const Json::Array &nodes);

template<typename Object>
using Dict = std::map<std::string, const Object *>;

using StopsDict = Dict<Stop>;
using BusesDict = Dict<Bus>;
}
