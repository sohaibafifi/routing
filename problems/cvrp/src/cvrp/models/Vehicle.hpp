//
// Created by Sohaib LAFIFI on 21/11/2019.
//

#pragma once

#include <vrp/models/Client.hpp>
#include <plugins/attributes/CapacityPlugin/Stock.hpp>
#include <core/interfaces/IAttribute.hpp>
#include <plugins/attributes/ComposableCorePlugin/models/Vehicle.hpp>

namespace cvrp {
    namespace models {
        struct Vehicle : public routing::models::Vehicle, public routing::attributes::Stock {
            Vehicle(unsigned id, routing::Capacity capacity)
                    : routing::models::Vehicle(id),
                      routing::attributes::Stock(capacity) {
            }
        };
    }
}