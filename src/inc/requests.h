#ifndef __REQUESTS_H__
#define __REQUESTS_H__
#pragma once

#include "json.h"
#include "transport_catalog.h"

#include <string>
#include <variant>


namespace Requests {

    struct Stop {
        std::string name;

        Json::Dict process(TransportCatalog const & db) const;
    };

    struct Bus {
        std::string name;

        Json::Dict process(TransportCatalog const & db) const;
    };

    struct Route {
        std::string stop_from;
        std::string stop_to;

        Json::Dict process(TransportCatalog const & db) const;
    };

    std::variant<Stop, Bus, Route> read(Json::Dict const & attrs);

    std::vector<Json::Node> processAll(TransportCatalog const & db, std::vector<Json::Node> const & requests);
}

#endif /* __REQUESTS_H__ */
