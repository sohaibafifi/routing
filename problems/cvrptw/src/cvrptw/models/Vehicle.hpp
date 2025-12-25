//
// Created by Sohaib LAFIFI on 21/11/2019.
//

#pragma once

#include <cvrp/models/Vehicle.hpp>
#include <plugins/attributes/CapacityPlugin/Stock.hpp>
#include <core/interfaces/IAttribute.hpp>
#include <plugins/attributes/ComposableCorePlugin/models/Vehicle.hpp>

namespace cvrptw {
    namespace models {
        struct Vehicle : public cvrp::models::Vehicle{
            Vehicle(unsigned id, routing::Capacity capacity)
                    : cvrp::models::Vehicle(id, capacity) {
            }
        };
    }
}