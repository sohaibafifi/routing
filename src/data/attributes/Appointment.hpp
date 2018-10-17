#pragma once
#include "../attributes.hpp"
#include "Rendezvous.hpp"
namespace routing {
    namespace attributes {

        struct Appointment {
                Appointment(const Duration& p_maxshift, const Duration& p_wait)
                    : maxshift(p_maxshift), wait(p_wait) {}
                SolutionValue<Duration> maxshift;
                SolutionValue<Duration> wait;
                Duration getMaxshift() const { return this->maxshift.getValue(); }
                void setMaxshift(Duration maxshift) { this->maxshift.setValue(maxshift); }
                Duration getWait() const { return this->wait.getValue(); }
                void setWait(Duration wait) { this->wait.setValue(wait); }
        };

    }  // namespace attributes
}  // namespace routing
