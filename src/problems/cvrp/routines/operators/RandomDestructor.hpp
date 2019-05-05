#pragma once
#include "../../models/Solution.hpp"
#include "../../../../routines/operators/Destructor.hpp"
#include "../../../../routines/operators/RandomDestructionParameters.hpp"
namespace CVRP{
class RandomDestructor : public routing::Destructor
{
    public:
        virtual void destruct(routing::models::Solution *solution, routing::DestructionParameters* param) override;

};
}
