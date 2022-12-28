//
// Created by Sohaib LAFIFI on 21/11/2019.
//

#pragma once

#include <cvrptw/models/Client.hpp>
#include <core/data/attributes/Pickup.hpp>
#include <core/data/attributes/Delivery.hpp>
#include <core/data/attributes/GeoNode.hpp>
#include <core/data/attributes.hpp>

namespace pdvrptw {
    namespace models {
        struct Client :
                public cvrptw::models::Client,
                public routing::attributes::Pickup,
                public routing::attributes::Delivery{
            Client(unsigned id, const routing::Duration &x, const routing::Duration &y,
                   const routing::Duration &p_service, const routing::TW &timewindow,
                   const routing::PickupDemand &pickupDemand,
                   const routing::DeliveryDemand &deliveryDemand) :
                    cvrptw::models::Client(id, x, y, pickupDemand + deliveryDemand, p_service, timewindow),
                    routing::attributes::Pickup(pickupDemand),
                    routing::attributes::Delivery(deliveryDemand) {
            }

            explicit Client(cvrptw::models::Client *client) :
                    cvrptw::models::Client(*client),
                   routing::attributes::Pickup(client->getDemand()),
                    routing::attributes::Delivery(0)
                   {
            }
        };
    }
}