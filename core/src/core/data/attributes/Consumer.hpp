// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/data/attributes.hpp"

namespace routing {
    typedef double Demand;
    namespace attributes {

        /**
 * @brief a node with demand
 *
 */
        struct Consumer {
            explicit Consumer(const Demand &p_demand) : demand(p_demand), consumption(0) {}

            EntityData <Demand> demand;
            SolutionValue <Demand> consumption;

            Demand getConsumption() const { return this->consumption.getValue(); }

            void setConsumption(Demand consumption) { this->consumption.setValue(consumption); }

            Demand getDemand() const { return this->demand.getValue(); }
        };

    }
}
