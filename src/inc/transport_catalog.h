#ifndef __TRANSPORT_CATALOG_H__
#define __TRANSPORT_CATALOG_H__
#pragma once

#include "descriptions.h"
#include "json.h"
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
        int road_route_length = 0;
        double geo_route_length = 0.;
    };
}

class TransportCatalog {
private:
    using Bus = Responses::Bus;
    using Stop = Responses::Stop;

public:
    TransportCatalog(std::vector<Descriptions::InputQuery> data, Json::Dict const & routing_settings_json);

    Stop const * getStop(std::string const & name) const;

    Bus const * getBus(std::string const & name) const;

    std::optional<TransportRouter::RouteInfo>
    findRoute(const std::string & stop_from, std::string const & stop_to) const;

    std::string renderMap() const;

private:
    static int computeRoadRouteLength(
        std::vector<std::string> const & stops,
        Descriptions::StopsDict const & stops_dict
    );

    static double computeGeoRouteDistance(
        std::vector<std::string> const & stops,
        Descriptions::StopsDict const & stops_dict
    );

    std::unordered_map<std::string, Stop> stops_;
    std::unordered_map<std::string, Bus> buses_;
    std::unique_ptr<TransportRouter> router_;
};

#endif /* __TRANSPORT_CATALOG_H__ */
