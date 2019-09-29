
#include "RemoveCost.hpp"

CVRP::RemoveCost::RemoveCost(routing::Duration delta) : delta(delta) {}

routing::Duration CVRP::RemoveCost::getDelta() const {
    return delta;
}

void CVRP::RemoveCost::setDelta(routing::Duration delta) {
    RemoveCost::delta = delta;
}

bool CVRP::RemoveCost::operator>(const CVRP::RemoveCost &rhs) const {
    return delta > rhs.delta;
}

CVRP::RemoveCost::RemoveCost(const CVRP::RemoveCost &cost): delta(cost.getDelta()){}
