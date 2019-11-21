//
// Created by Sohaib LAFIFI on 21/11/2019.
//

#pragma once

#include <vrp/models/Client.hpp>
#include <core/data/attributes/Consumer.hpp>
#include <core/data/attributes.hpp>

namespace cvrp {
    namespace models {
        struct Client : public vrp::models::Client, public routing::attributes::Consumer {
            Client(unsigned id, const routing::Duration &x, const routing::Duration &y, const routing::Demand &demand) :
                    vrp::models::Client(id, x, y),
                    routing::attributes::Consumer(demand) {
            }
        };
    }
}