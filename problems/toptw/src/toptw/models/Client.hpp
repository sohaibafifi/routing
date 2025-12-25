//
// Created by Sohaib LAFIFI on 21/11/2019.
//

#pragma once

#include <cvrptw/models/Client.hpp>
#include <plugins/attributes/ProfitPlugin/Profiter.hpp>
#include <plugins/attributes/RoutingPlugin/GeoNode.hpp>
#include <core/interfaces/IAttribute.hpp>

namespace toptw {
    namespace models {
        struct Client :
                public cvrptw::models::Client,
                public routing::attributes::Profiter {
            Client(unsigned id, const routing::Duration &x, const routing::Duration &y, const routing::Demand &demand,
                   const routing::Duration &p_service, const routing::TW &timewindow, const routing::Profit &profit) :
                    cvrptw::models::Client(id, x, y, demand, p_service, timewindow),
                    routing::attributes::Profiter(profit) {
            }

            explicit Client(cvrptw::models::Client *client) :
                    cvrptw::models::Client(*client),
                   routing::attributes::Profiter(client->getDemand()) {
            }
        };
    }
}