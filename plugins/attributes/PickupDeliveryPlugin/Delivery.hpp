// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/interfaces/IAttribute.hpp"
#include <memory>

namespace routing {
    typedef int DeliveryDemand;
    namespace attributes {

        /**
         * @brief Delivery demand at a node
         *
         * Delivery models goods to be dropped off at this node,
         * reducing the vehicle's current load.
         */
        struct Delivery : public Attribute<Delivery> {
            explicit Delivery(const DeliveryDemand &p_delivery) : deliveryDemand(p_delivery) {}

            EntityData<DeliveryDemand> deliveryDemand;

            DeliveryDemand getDelivery() const { return this->deliveryDemand.getValue(); }

            // Legacy method name for backward compatibility
            DeliveryDemand getPickup() const { return getDelivery(); }

            // IAttribute implementation
            std::unique_ptr<IAttribute> clone() const override {
                return std::make_unique<Delivery>(getDelivery());
            }

            std::string name() const override {
                return "Delivery";
            }
        };

    }
}
