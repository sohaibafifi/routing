//
// Created by ali on 3/28/19.
//

#ifndef HYBRID_DESTRUCTOR_H
#define HYBRID_DESTRUCTOR_H


#include "../../models/Solution.hpp"
#include "../../../cvrp/routines/operators/Destructor.hpp"

namespace CVRPTW{
    class Destructor : public CVRP::Destructor
    {
    public:
        virtual void destruct(routing::models::Solution *solution, unsigned long n, DestructionPolicy policy) override;

        virtual void sequentialDestruction(routing::models::Solution* solution, unsigned long position, unsigned long length);
        virtual void randomDestructoion(routing::models::Solution* solution, unsigned long n) override;
        virtual void worstCostDestruction(routing::models::Solution* solution);
    };



}

#endif //HYBRID_DESTRUCTOR_H
