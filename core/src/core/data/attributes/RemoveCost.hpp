// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#ifndef HYBRID_REMOVECOST_HPP

#define HYBRID_REMOVECOST_HPP

#include "GeoNode.hpp"

namespace routing {
    class RemoveCost {

    private:
        routing::Duration delta;
    public:
        bool operator>(const RemoveCost &rhs) const {
            return delta > rhs.delta;
        }


        routing::Duration getDelta() const {
            return delta;
        }

        void setDelta(routing::Duration delta) {
            RemoveCost::delta = delta;
        }


        explicit RemoveCost(routing::Duration delta) : delta(delta) {}

        RemoveCost() : delta(0) {}

        RemoveCost(const RemoveCost &cost) : delta(cost.getDelta()) {}
    };
}


#endif //HYBRID_REMOVECOST_HPP
