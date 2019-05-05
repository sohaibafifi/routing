//
// Created by ali on 5/5/19.
//

#ifndef HYBRID_SEQUENTIALDESTRUCTOR_HPP
#define HYBRID_SEQUENTIALDESTRUCTOR_HPP

#include "../../../../routines/operators/Destructor.hpp"
#include "../../../../routines/operators/SequentialDestructionParameters.hpp"


namespace CVRPTW{
    class SequentialDestructor : public routing::Destructor{
        virtual void destruct(routing::models::Solution *solution, routing::DestructionParameters* param) override;
    };

}


#endif //HYBRID_SEQUENTIALDESTRUCTOR_HPP
