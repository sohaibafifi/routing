#pragma once
#include <sstream>

#include "../../../data/models/Vehicle.hpp"
#include "../../../data/attributes/Stock.hpp"

namespace CVRP
{
    struct Vehicle
            : public routing::models::Vehicle, public routing::attributes::Stock
    {
            Vehicle(unsigned id, routing::Capacity capacity)
                : routing::attributes::Stock(capacity)
            {
                this->setID(id);
            }
    };
}
