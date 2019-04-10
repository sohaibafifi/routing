#pragma once
#include "../../models/Solution.hpp"
#include "../../../../routines/operators/Constructor.hpp"
#include "../../models/InsertionCost.hpp"
namespace CVRP {

    class Constructor : public routing::Constructor
    {
        public:
            virtual bool bestInsertion(routing::models::Solution * solution) override;
    };

}
