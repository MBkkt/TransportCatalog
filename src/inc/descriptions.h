#ifndef __DESCRIPTIONS_H__
#define __DESCRIPTIONS_H__
#pragma once

#include "json.h"
#include "sphere.h"

#include <string>
#include <unordered_map>
#include <variant>
#include <vector>


namespace Descriptions {
    struct Stop {
        std::string name;
        Sphere::Point position;
        std::unordered_map<std::string, int> distances;

        static Stop parseFrom(Json::Dict const & attrs);
    };

    int computeStopsDistance(Stop const & lhs, Stop const & rhs);

    std::vector<std::string> parseStops(std::vector<Json::Node> const & stop_nodes, bool is_roundtrip);

    struct Bus {
        std::string name;
        std::vector<std::string> stops;

        static Bus parseFrom(Json::Dict const & attrs);
    };

    using InputQuery = std::variant<Stop, Bus>;

    std::vector<InputQuery> readDescriptions(std::vector<Json::Node> const & nodes);

    template<typename Object>
    using Dict = std::unordered_map<std::string, const Object *>;

    using StopsDict = Dict<Stop>;
    using BusesDict = Dict<Bus>;
}

#endif /* __DESCRIPTIONS_H__ */
