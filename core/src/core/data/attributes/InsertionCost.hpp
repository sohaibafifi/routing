//
// Created by ali on 4/10/19.
//

#ifndef HYBRID_INSERTIONCOST_HPP

#define HYBRID_INSERTIONCOST_HPP

#include "GeoNode.hpp"

namespace routing {
    class InsertionCost {
    private:
        routing::Duration delta;
        bool possible;
    public:
        bool operator>(const InsertionCost &rhs) const{
            return delta > rhs.delta;
        }


        routing::Duration getDelta() const {
            return delta;
        }

        void setDelta(routing::Duration delta){
            InsertionCost::delta = delta;
        }

        bool isPossible() const{
            return possible;
        }

        void setPossible(bool possible){
            InsertionCost::possible = possible;
        }

        InsertionCost(routing::Duration delta, bool possible) : delta(delta), possible(possible) {}
        InsertionCost() : delta(0), possible(true) {}

        InsertionCost(const InsertionCost &cost): delta(cost.getDelta()), possible(cost.isPossible()){}
    };
}


#endif //HYBRID_INSERTIONCOST_HPP
