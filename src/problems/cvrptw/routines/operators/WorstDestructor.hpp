//
// Created by ali on 5/5/19.
//

#ifndef HYBRID_WORSTDESTRUCTION_HPP
#define HYBRID_WORSTDESTRUCTION_HPP

#include "../../../../routines/operators/Destructor.hpp"
namespace CVRPTW{
    class WorstDestructor :  public routing::Destructor{
        virtual void destruct(routing::models::Solution* solution, routing::DestructionParameters* parameters) override;
    };

}

#endif //HYBRID_WORSTDESTRUCTION_HPP
