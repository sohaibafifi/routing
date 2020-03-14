// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/data/attributes.hpp"

namespace routing {
    typedef double Capacity;
    namespace attributes {

        /**
         * @brief
         */
        struct Stock {
            explicit Stock(const Capacity &p_capacity) : capacity(p_capacity) {}

            EntityData <Capacity> capacity;

            Capacity getCapacity() const { return this->capacity.getValue(); }
        };
    }
}
