//
// Created by Sohaib on 09/29/19.
//

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


        RemoveCost(routing::Duration delta) : delta(delta) {}

        RemoveCost() : delta(0) {}

        RemoveCost(const RemoveCost &cost) : delta(cost.getDelta()) {}
    };
}


#endif //HYBRID_REMOVECOST_HPP
