// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/data/attributes.hpp"

namespace routing {
    typedef int PickupDemand;
    namespace attributes {

        /**
 * @brief a node with a pickup
 *
 */
        struct Pickup {
            explicit Pickup(const PickupDemand &p_pickup) : pickup(p_pickup) {}

            EntityData <PickupDemand> pickup;

            PickupDemand getPickup() const { return this->pickup.getValue(); }
        };

    }
}
