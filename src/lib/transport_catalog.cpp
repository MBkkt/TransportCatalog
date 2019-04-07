#include "transport_catalog.h"

#include <sstream>


using namespace std;

TransportCatalog::TransportCatalog(vector<Descriptions::InputQuery> data, Json::Dict const & routing_settings_json) {
    auto stops_end = partition(begin(data), end(data), [](auto const & item) {
        return holds_alternative<Descriptions::Stop>(item);
    });

    Descriptions::StopsDict stops_dict;
    for (auto const & item : Range {begin(data), stops_end}) {
        auto const & stop = get<Descriptions::Stop>(item);
        stops_dict[stop.name] = &stop;
        stops_.insert({stop.name, {}});
    }

    Descriptions::BusesDict buses_dict;
    for (auto const & item : Range {stops_end, end(data)}) {
        auto const & bus = get<Descriptions::Bus>(item);

        buses_dict[bus.name] = &bus;
        buses_[bus.name] = Bus {
            bus.stops.size(),
            computeUniqueItemsCount(asRange(bus.stops)),
            computeRoadRouteLength(bus.stops, stops_dict),
            computeGeoRouteDistance(bus.stops, stops_dict)
        };

        for (string const & stop_name : bus.stops) {
            stops_.at(stop_name).bus_names.insert(bus.name);
        }
    }

    router_ = make_unique<TransportRouter>(stops_dict, buses_dict, routing_settings_json);
}

TransportCatalog::Stop const * TransportCatalog::getStop(string const & name) const {
    return getValuePointer(stops_, name);
}

TransportCatalog::Bus const * TransportCatalog::getBus(string const & name) const {
    return getValuePointer(buses_, name);
}

optional<TransportRouter::RouteInfo>
TransportCatalog::findRoute(string const & stop_from, string const & stop_to) const {
    return router_->findRoute(stop_from, stop_to);
}

int TransportCatalog::computeRoadRouteLength(vector<string> const & stops,
                                             Descriptions::StopsDict const & stops_dict) {
    int result = 0;
    for (size_t i = 1; i < stops.size(); ++i) {
        result += Descriptions::computeStopsDistance(*stops_dict.at(stops[i - 1]), *stops_dict.at(stops[i]));
    }
    return result;
}

double TransportCatalog::computeGeoRouteDistance(vector<string> const & stops,
                                                 Descriptions::StopsDict const & stops_dict) {
    double result = 0;
    for (size_t i = 1; i < stops.size(); ++i) {
        result += Sphere::distance(
            stops_dict.at(stops[i - 1])->position, stops_dict.at(stops[i])->position
        );
    }
    return result;
}
