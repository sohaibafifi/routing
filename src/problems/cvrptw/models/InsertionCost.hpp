//
// Created by ali on 4/10/19.
//

#pragma once

#include "../../cvrp/models/InsertionCost.hpp"

namespace CVRPTW{
    class InsertionCost: public CVRP::InsertionCost  {

    public:

        routing::Duration getShift() const {
            return shift;
        }

        void setShift(routing::Duration shift) {
            this->shift = shift;
        }

        InsertionCost(routing::Duration delta, bool possible, routing::Duration shift) : CVRP::InsertionCost(
            delta, possible), shift(shift) {}

    private:
        routing::Duration shift;
    };
}

