#pragma once
#include "../../models/Solution.hpp"
#include "../../../../routines/operators/Destructor.hpp"
namespace CVRP{
class Destructor : public routing::Destructor
{
    public:
        virtual void destruct(routing::models::Solution *solution, unsigned long n) override;
};
}
