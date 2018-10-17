#pragma once
#include "../../models/Solution.hpp"
#include "../../../../routines/operators/Constructor.hpp"

namespace CVRP {

    class Constructor : public routing::Constructor
    {
        public:
            virtual bool bestInsertion(routing::models::Solution * solution) override;
    };

}
