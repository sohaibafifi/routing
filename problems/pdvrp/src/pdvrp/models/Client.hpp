//
// Created by Sohaib LAFIFI on 21/11/2019.
//

#pragma once

#include <cvrp/models/Client.hpp>
#include <core/data/attributes/Pickup.hpp>
#include <core/data/attributes/Delivery.hpp>
#include <core/data/attributes/GeoNode.hpp>
#include <core/data/attributes.hpp>

namespace pdvrp {
    namespace models {
        struct Client :
                public cvrp::models::Client,
                public routing::attributes::Pickup,
                public routing::attributes::Delivery
                {
            Client(unsigned id, const routing::Duration &x, const routing::Duration &y,
                   const routing::PickupDemand &pickup,
                   const routing::DeliveryDemand &delivery
                   ) :
                    cvrp::models::Client(id, x, y, pickup + delivery),
                    routing::attributes::Pickup(pickup),
                    routing::attributes::Delivery(delivery){
            }

            explicit Client(cvrp::models::Client *client) :
                   cvrp::models::Client(*client),
                   routing::attributes::Pickup(client->getDemand()),
                    routing::attributes::Delivery(0)
                   {
            }
        };
    }
}