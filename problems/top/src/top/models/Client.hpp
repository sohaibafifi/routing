//
// Created by Sohaib LAFIFI on 21/11/2019.
//

#pragma once

#include <cvrp/models/Client.hpp>
#include <core/data/attributes/Profiter.hpp>
#include <core/data/attributes/GeoNode.hpp>
#include <core/data/attributes.hpp>

namespace top {
    namespace models {
        struct Client :
                public vrp::models::Client,
                public routing::attributes::Profiter {
            Client(unsigned id, const routing::Duration &x, const routing::Duration &y, const routing::Profit &profit) :
                    vrp::models::Client(id, x, y),
                    routing::attributes::Profiter(profit) {
            }

            explicit Client(cvrp::models::Client *client) :
                    vrp::models::Client(client->getID(),
                                           client->getX(),
                                           client->getY()),
                   routing::attributes::Profiter(client->getDemand()) {
            }
        };
    }
}