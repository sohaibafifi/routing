#pragma once
#include "../../models/Solution.hpp"
#include "../../../../routines/operators/Destructor.hpp"
namespace CVRP{
class Destructor : public routing::Destructor
{
    public:
        virtual void destruct(routing::models::Solution *solution, unsigned long n, DestructionPolicy policy) override;

        virtual void randomDestructoion(routing::models::Solution* solution, unsigned long n);
};
}
