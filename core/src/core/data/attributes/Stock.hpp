// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/data/attributes.hpp"
#include <memory>

namespace routing {
    typedef double Capacity;
    namespace attributes {

        /**
         * @brief Vehicle capacity attribute
         *
         * Stock models the capacity of a vehicle, limiting how much
         * demand it can satisfy in a single route.
         */
        struct Stock : public Attribute<Stock> {
            explicit Stock(const Capacity &p_capacity) : capacity(p_capacity) {}

            EntityData<Capacity> capacity;

            Capacity getCapacity() const { return this->capacity.getValue(); }

            // IAttribute implementation
            std::unique_ptr<IAttribute> clone() const override {
                return std::make_unique<Stock>(getCapacity());
            }

            std::string name() const override {
                return "Stock";
            }
        };
    }
}
