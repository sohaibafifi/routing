//
// Created by ali on 6/26/19.
//

#ifndef HYBRID_DESTRUCTOR_TSPTWH_HPP
#define HYBRID_DESTRUCTOR_TSPTWH_HPP

#include "../../../../routines/operators/DestructionParameters.hpp"
#include "../../models/Tour.hpp"

namespace CVRPTW{
    class Destructor_TSPTWH {
        public:
            virtual void destruct(routing::models::Solution *solution, routing::DestructionParameters* param);
    };
}



#endif //HYBRID_DESTRUCTOR_TSPTWH_HPP
