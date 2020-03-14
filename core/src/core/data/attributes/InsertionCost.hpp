// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once
#include "GeoNode.hpp"

namespace routing {
    class InsertionCost {
    private:
        routing::Duration delta;
        bool possible;
    public:
        bool operator>(const InsertionCost &rhs) const {
            return delta > rhs.delta;
        }


        routing::Duration getDelta() const {
            return delta;
        }

        void setDelta(routing::Duration delta) {
            InsertionCost::delta = delta;
        }

        bool isPossible() const {
            return possible;
        }

        void setPossible(bool possible) {
            InsertionCost::possible = possible;
        }

        InsertionCost(routing::Duration delta, bool possible) : delta(delta), possible(possible) {}

        InsertionCost() : delta(0), possible(true) {}

        InsertionCost(const InsertionCost &cost) : delta(cost.getDelta()), possible(cost.isPossible()) {}
    };
}