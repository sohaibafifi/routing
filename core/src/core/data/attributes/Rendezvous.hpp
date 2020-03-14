// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/data/attributes.hpp"
#include "ServiceQuery.hpp"

namespace routing {
    typedef std::pair<Duration, Duration> TW;

    namespace attributes {

        /**
 * @brief a node with a time window
 *
 */
        struct Rendezvous {
            Rendezvous(const TW &timewindow) : tw(timewindow) {}

            EntityData<TW> tw;

            TW getTw() const { return tw.getValue(); }

            Duration getTwOpen() const { return this->tw.getValue().first; }

            Duration getTwClose() const { return this->tw.getValue().second; }

            Duration getEST() const { return this->getTwOpen(); }

            Duration getLST() const { return this->getTwClose(); }
        };

    }
}
