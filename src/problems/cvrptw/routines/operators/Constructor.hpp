//
// Created by ali on 3/28/19.
//

#ifndef HYBRID_CONSTRUCTOR_H
#define HYBRID_CONSTRUCTOR_H

#include "../../models/Tour.hpp"
#include "../../../cvrp/routines/operators/Constructor.hpp"
#include "../../models/Solution.hpp"
namespace CVRPTW{
    class Constructor : public CVRP::Constructor{
        public:
            virtual bool bestInsertion(routing::models::Solution * solution) override;
    };

}

#endif //HYBRID_CONSTRUCTOR_H
