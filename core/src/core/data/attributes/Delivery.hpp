// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/data/attributes.hpp"

namespace routing {
    typedef int DeliveryDemand;
    namespace attributes {

        /**
         * @brief a node with delivery
         *
         */
        struct Delivery {
            explicit Delivery(const DeliveryDemand &p_delivery) : deliveryDemand(p_delivery) {}

            EntityData <DeliveryDemand> deliveryDemand;

            DeliveryDemand getPickup() const { return this->deliveryDemand.getValue(); }
        };

    }
}
