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
        virtual void destruct(routing::models::Solution *solution, unsigned long n) override;

        virtual void SequentialDestruction(routing::models::Solution* solution, unsigned long position, unsigned long length);
    };


    enum DestructionPolicy{
        RANDOM,
        SEQUENTIAL,
        WORST
    };
}

#endif //HYBRID_DESTRUCTOR_H
