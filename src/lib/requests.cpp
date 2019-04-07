#include "requests.h"
#include "transport_router.h"

#include <vector>


using namespace std;

namespace Requests {

    Json::Dict Stop::process(TransportCatalog const & db) const {
        auto const * stop = db.getStop(name);
        Json::Dict dict;
        if (!stop) {
            dict["error_message"] = Json::Node("not found"s);
        } else {
            vector<Json::Node> bus_nodes;
            bus_nodes.reserve(stop->bus_names.size());
            for (auto const & bus_name : stop->bus_names) {
                bus_nodes.emplace_back(bus_name);
            }
            dict["buses"] = Json::Node(move(bus_nodes));
        }
        return dict;
    }

    Json::Dict Bus::process(TransportCatalog const & db) const {
        auto const * bus = db.getBus(name);
        Json::Dict dict;
        if (!bus) {
            dict["error_message"] = Json::Node("not found"s);
        } else {
            dict = {
                {"stop_count",        Json::Node(static_cast<int>(bus->stop_count))},
                {"unique_stop_count", Json::Node(static_cast<int>(bus->unique_stop_count))},
                {"route_length",      Json::Node(bus->road_route_length)},
                {"curvature",         Json::Node(bus->road_route_length / bus->geo_route_length)},
            };
        }
        return dict;
    }

    struct RouteItemResponseBuilder {
        Json::Dict operator()(TransportRouter::RouteInfo::BusItem const & bus_item) const {
            return Json::Dict {
                {"type",       Json::Node("Bus"s)},
                {"bus",        Json::Node(bus_item.bus_name)},
                {"time",       Json::Node(bus_item.time)},
                {"span_count", Json::Node(static_cast<int>(bus_item.span_count))}
            };
        }

        Json::Dict operator()(TransportRouter::RouteInfo::WaitItem const & wait_item) const {
            return Json::Dict {
                {"type",      Json::Node("Wait"s)},
                {"stop_name", Json::Node(wait_item.stop_name)},
                {"time",      Json::Node(wait_item.time)},
            };
        }
    };

    Json::Dict Route::process(TransportCatalog const & db) const {
        Json::Dict dict;
        auto const route = db.findRoute(stop_from, stop_to);
        if (!route) {
            dict["error_message"] = Json::Node("not found"s);
        } else {
            dict["total_time"] = Json::Node(route->total_time);
            vector<Json::Node> items;
            items.reserve(route->items.size());
            for (const auto & item : route->items) {
                items.emplace_back(visit(RouteItemResponseBuilder {}, item));
            }

            dict["items"] = move(items);
        }

        return dict;
    }

    variant<Stop, Bus, Route> read(Json::Dict const & attrs) {
        const string & type = attrs.at("type").asString();
        if (type == "Bus") {
            return Bus {attrs.at("name").asString()};
        } else if (type == "Stop") {
            return Stop {attrs.at("name").asString()};
        } else {
            return Route {attrs.at("from").asString(), attrs.at("to").asString()};
        }
    }

    vector<Json::Node> processAll(TransportCatalog const & db, vector<Json::Node> const & requests) {
        vector<Json::Node> responses;
        responses.reserve(requests.size());
        for (Json::Node const & request_node : requests) {
            Json::Dict dict = visit(
                [&db](auto const & request) {
                    return request.process(db);
                },
                Requests::read(request_node.asMap())
            );
            dict["request_id"] = Json::Node(request_node.asMap().at("id").asInt());
            responses.emplace_back(dict);
        }
        return responses;
    }

}
