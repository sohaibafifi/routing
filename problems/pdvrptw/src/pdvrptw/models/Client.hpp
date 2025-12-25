//
// Created by Sohaib LAFIFI on 21/11/2019.
//

#pragma once

#include <cvrptw/models/Client.hpp>
#include <plugins/attributes/PickupDeliveryPlugin/Pickup.hpp>
#include <plugins/attributes/PickupDeliveryPlugin/Delivery.hpp>
#include <plugins/attributes/RoutingPlugin/GeoNode.hpp>
#include <core/interfaces/IAttribute.hpp>

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