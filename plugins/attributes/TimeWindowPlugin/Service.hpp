// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/interfaces/IAttribute.hpp"
#include "ServiceQuery.hpp"
#include <memory>

namespace routing {

    namespace attributes {

        /**
         * @brief Service start time (solution value)
         *
         * Service stores the actual start time of service at a node
         * in the current solution. This is a mutable solution value.
         */
        struct Service : public Attribute<Service> {
            Service(const Duration &p_start) : start(p_start) {}

            SolutionValue<Duration> start;

            Duration getStart() const { return this->start.getValue(); }
            void setStart(Duration start) { this->start.setValue(start); }

            // IAttribute implementation
            std::unique_ptr<IAttribute> clone() const override {
                return std::make_unique<Service>(getStart());
            }

            std::string name() const override {
                return "Service";
            }
        };

    }
}
