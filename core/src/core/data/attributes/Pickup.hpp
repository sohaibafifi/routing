// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/data/attributes.hpp"
#include <memory>

namespace routing {
    typedef int PickupDemand;
    namespace attributes {

        /**
         * @brief Pickup demand at a node
         *
         * Pickup models goods to be collected from this node,
         * adding to the vehicle's current load.
         */
        struct Pickup : public Attribute<Pickup> {
            explicit Pickup(const PickupDemand &p_pickup) : pickup(p_pickup) {}

            EntityData<PickupDemand> pickup;

            PickupDemand getPickup() const { return this->pickup.getValue(); }

            // IAttribute implementation
            std::unique_ptr<IAttribute> clone() const override {
                return std::make_unique<Pickup>(getPickup());
            }

            std::string name() const override {
                return "Pickup";
            }
        };

    }
}
