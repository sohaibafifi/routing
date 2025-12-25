//
// Created by Sohaib LAFIFI on 21/11/2019.
//

#pragma once

#include <cvrp/models/Client.hpp>
#include <plugins/attributes/PickupDeliveryPlugin/Pickup.hpp>
#include <plugins/attributes/PickupDeliveryPlugin/Delivery.hpp>
#include <plugins/attributes/RoutingPlugin/GeoNode.hpp>
#include <core/interfaces/IAttribute.hpp>

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