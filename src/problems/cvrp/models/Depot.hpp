#pragma once
#include <sstream>

#include "../../../data/models/Depot.hpp"
#include "../../../data/attributes/Consumer.hpp"
#include "../../../data/attributes/GeoNode.hpp"

namespace CVRP
{
    struct Depot
            : public routing::models::Depot, public routing::attributes::GeoNode
    {
            Depot(unsigned id, const routing::Duration &x, const routing::Duration &y)
                : routing::models::Depot(id),
                  routing::attributes::GeoNode(x, y)
            {
            }
    };

}
