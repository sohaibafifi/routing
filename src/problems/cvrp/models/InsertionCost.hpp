//
// Created by ali on 4/10/19.
//
#pragma once

#include "../../../data/attributes/InsertionCost.hpp"


namespace CVRP {
    class InsertionCost : public routing::InsertionCost {
        private:
            routing::Duration delta;
            bool possible;
    public:
        bool operator>(const InsertionCost &rhs) const;

        routing::Duration getDelta() const;

        void setDelta(routing::Duration delta);

        bool isPossible() const;

        void setPossible(bool possible);

        InsertionCost(routing::Duration delta, bool possible);

        InsertionCost(const InsertionCost &cost);



    };

}


