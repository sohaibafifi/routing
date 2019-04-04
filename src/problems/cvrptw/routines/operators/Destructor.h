//
// Created by ali on 3/28/19.
//

#ifndef HYBRID_DESTRUCTOR_H
#define HYBRID_DESTRUCTOR_H


#include "../../models/Solution.h"
#include "../../../cvrp/routines/operators/Destructor.hpp"

namespace CVRPTW{
    class Destructor : public CVRP::Destructor
    {
    public:
        virtual void destruct(routing::models::Solution *solution, unsigned long n) override;
    };
}

#endif //HYBRID_DESTRUCTOR_H
