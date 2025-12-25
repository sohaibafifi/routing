// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/interfaces/IAttribute.hpp"
#include "ServiceQuery.hpp"
#include <memory>
#include <utility>

namespace routing {
    typedef std::pair<Duration, Duration> TW;

    namespace attributes {

        /**
         * @brief Time window constraint for a node
         *
         * Rendezvous models a time window within which service
         * must begin at this node.
         */
        struct Rendezvous : public Attribute<Rendezvous> {
            Rendezvous(const TW &timewindow) : tw(timewindow) {}
            Rendezvous(const Duration &twOpen, const Duration &twClose)
                : tw(std::make_pair(twOpen, twClose)) {}

            EntityData<TW> tw;

            TW getTw() const { return tw.getValue(); }
            Duration getTwOpen() const { return this->tw.getValue().first; }
            Duration getTwClose() const { return this->tw.getValue().second; }
            Duration getEST() const { return this->getTwOpen(); }
            Duration getLST() const { return this->getTwClose(); }

            /// Check if a given time falls within the time window
            bool isWithinWindow(Duration time) const {
                return time >= getTwOpen() && time <= getTwClose();
            }

            /// Get the width of the time window
            Duration getWindowWidth() const {
                return getTwClose() - getTwOpen();
            }

            // IAttribute implementation
            std::unique_ptr<IAttribute> clone() const override {
                return std::make_unique<Rendezvous>(getTw());
            }

            std::string name() const override {
                return "Rendezvous";
            }
        };

    }
}
