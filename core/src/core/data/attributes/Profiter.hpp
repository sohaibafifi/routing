// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/data/attributes.hpp"

namespace routing {
    typedef double Profit;
    namespace attributes {


        /**
 * @brief a node with a profit property
 *
 */
        struct Profiter {
            Profiter(const Profit &p_profit) : profit(p_profit) {}

            EntityData<Profit> profit;

            double getProfit() const { return this->profit.getValue(); }
        };

    }
}
