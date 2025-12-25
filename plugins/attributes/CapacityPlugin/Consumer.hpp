// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/interfaces/IAttribute.hpp"
#include <memory>

namespace routing {
    typedef int Demand;
    namespace attributes {

        /**
         * @brief A node with demand (capacity consumption)
         *
         * Consumer models the demand that must be satisfied at a node,
         * consuming vehicle capacity.
         */
        struct Consumer : public Attribute<Consumer> {
            explicit Consumer(const Demand &p_demand) : demand(p_demand), consumption(0) {}

            EntityData<Demand> demand;
            SolutionValue<Demand> consumption;

            Demand getDemand() const { return this->demand.getValue(); }
            Demand getConsumption() const { return this->consumption.getValue(); }
            void setConsumption(Demand consumption) { this->consumption.setValue(consumption); }

            // IAttribute implementation
            std::unique_ptr<IAttribute> clone() const override {
                auto copy = std::make_unique<Consumer>(getDemand());
                copy->setConsumption(getConsumption());
                return copy;
            }

            std::string name() const override {
                return "Consumer";
            }
        };

    }
}
