// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/interfaces/IAttribute.hpp"
#include "Rendezvous.hpp"
#include <memory>

namespace routing {
    namespace attributes {

        /**
         * @brief Appointment flexibility metrics (solution values)
         *
         * Appointment stores solution-dependent values related to
         * time window flexibility: maximum shift and waiting time.
         */
        struct Appointment : public Attribute<Appointment> {
            Appointment(const Duration &p_maxshift, const Duration &p_wait)
                    : maxshift(p_maxshift), wait(p_wait) {}

            SolutionValue<Duration> maxshift;
            SolutionValue<Duration> wait;

            Duration getMaxshift() const { return this->maxshift.getValue(); }
            void setMaxshift(Duration maxshift) { this->maxshift.setValue(maxshift); }

            Duration getWait() const { return this->wait.getValue(); }
            void setWait(Duration wait) { this->wait.setValue(wait); }

            // IAttribute implementation
            std::unique_ptr<IAttribute> clone() const override {
                return std::make_unique<Appointment>(getMaxshift(), getWait());
            }

            std::string name() const override {
                return "Appointment";
            }
        };

    }  // namespace attributes
}  // namespace routing
