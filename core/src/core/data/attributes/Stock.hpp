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
