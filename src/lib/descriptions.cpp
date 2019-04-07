#include "descriptions.h"


using namespace std;

namespace Descriptions {

    Stop Stop::parseFrom(Json::Dict const & attrs) {
        Stop stop = {
            .name = attrs.at("name").asString(),
            .position = {
                .latitude = attrs.at("latitude").asDouble(),
                .longitude = attrs.at("longitude").asDouble(),
            }
        };
        if (attrs.count("road_distances") > 0) {
            for (auto const &[neighbour_stop, distance_node] : attrs.at("road_distances").asMap()) {
                stop.distances[neighbour_stop] = distance_node.asInt();
            }
        }
        return stop;
    }

    vector<string> parseStops(vector<Json::Node> const & stop_nodes, bool is_roundtrip) {
        vector<string> stops;
        stops.reserve(stop_nodes.size());
        for (const Json::Node & stop_node : stop_nodes) {
            stops.push_back(stop_node.asString());
        }
        if (is_roundtrip || stops.size() <= 1) {
            return stops;
        }
        stops.reserve(stops.size() * 2 - 1);  // end stop is not repeated
        for (size_t stop_idx = stops.size() - 1; stop_idx > 0; --stop_idx) {
            stops.push_back(stops[stop_idx - 1]);
        }
        return stops;
    }

    int computeStopsDistance(Stop const & lhs, Stop const & rhs) {
        if (auto it = lhs.distances.find(rhs.name); it != lhs.distances.end()) {
            return it->second;
        } else {
            return rhs.distances.at(lhs.name);
        }
    }

    Bus Bus::parseFrom(const Json::Dict & attrs) {
        return Bus {
            .name = attrs.at("name").asString(),
            .stops = parseStops(attrs.at("stops").asArray(), attrs.at("is_roundtrip").asBool()),
        };
    }

    vector<InputQuery> readDescriptions(vector<Json::Node> const & nodes) {
        vector<InputQuery> result;
        result.reserve(nodes.size());

        for (Json::Node const & node : nodes) {
            auto const & node_dict = node.asMap();
            if (node_dict.at("type").asString() == "Bus") {
                result.emplace_back(Bus::parseFrom(node_dict));
            } else {
                result.emplace_back(Stop::parseFrom(node_dict));
            }
        }

        return result;
    }

}
