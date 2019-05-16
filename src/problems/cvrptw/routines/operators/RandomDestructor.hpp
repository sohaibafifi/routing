//
// Created by ali on 5/5/19.
//

#pragma once

#include "../../../../routines/operators/Destructor.hpp"
#include "../../../../mtrand.hpp"
namespace CVRPTW{
    class RandomDestructor : public routing::Destructor {

        virtual void destruct(routing::models::Solution *solution, routing::DestructionParameters* param) override;

    };

}
