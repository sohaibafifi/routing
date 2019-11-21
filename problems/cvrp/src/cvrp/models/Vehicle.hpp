//
// Created by Sohaib LAFIFI on 21/11/2019.
//

#pragma once

#include <vrp/models/Client.hpp>
#include <core/data/attributes/Stock.hpp>
#include <core/data/attributes.hpp>
#include <core/data/models/Vehicle.hpp>

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