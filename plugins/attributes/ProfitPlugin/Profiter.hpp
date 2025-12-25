// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/interfaces/IAttribute.hpp"
#include <memory>

namespace routing {
    typedef double Profit;
    namespace attributes {

        /**
         * @brief Profit/reward for visiting a node
         *
         * Profiter models the benefit gained from visiting this node,
         * used in problems like TOP (Team Orienteering Problem).
         */
        struct Profiter : public Attribute<Profiter> {
            Profiter(const Profit &p_profit) : profit(p_profit) {}

            EntityData<Profit> profit;

            double getProfit() const { return this->profit.getValue(); }

            // IAttribute implementation
            std::unique_ptr<IAttribute> clone() const override {
                return std::make_unique<Profiter>(getProfit());
            }

            std::string name() const override {
                return "Profiter";
            }
        };

    }
}
