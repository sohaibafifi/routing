//
// Created by ali on 4/10/19.
//

#include "InsertionCost.hpp"

CVRP::InsertionCost::InsertionCost(routing::Duration delta, bool possible) : delta(delta), possible(possible) {}

routing::Duration CVRP::InsertionCost::getDelta() const {
    return delta;
}

void CVRP::InsertionCost::setDelta(routing::Duration delta) {
    InsertionCost::delta = delta;
}

bool CVRP::InsertionCost::isPossible() const {
    return possible;
}

void CVRP::InsertionCost::setPossible(bool possible) {
    InsertionCost::possible = possible;
}

bool CVRP::InsertionCost::operator>(const CVRP::InsertionCost &rhs) const {
    return delta > rhs.delta &&
           possible == rhs.possible;
}

CVRP::InsertionCost::InsertionCost(const CVRP::InsertionCost &cost): delta(cost.getDelta()), possible(cost.isPossible()){}
