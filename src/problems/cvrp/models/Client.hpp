#pragma once
#include <sstream>
#include "../../../data/models/Client.hpp"
#include "../../../data/attributes/Consumer.hpp"
#include "../../../data/attributes/GeoNode.hpp"

namespace CVRP
{
    struct Client
            : public routing::models::Client, public routing::attributes::Consumer, public routing::attributes::GeoNode
    {
            Client(unsigned id, routing::Demand demand, const routing::Duration &x, const routing::Duration &y)
                : routing::attributes::Consumer(demand),
                  routing::attributes::GeoNode(x, y)
            {
                this->setID(id);
            }
    };
}
