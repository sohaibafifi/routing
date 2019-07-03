//
// Created by ali on 6/26/19.
//

#ifndef HYBRID_CONSTRUCTOR_TSPTWH_HPP
#define HYBRID_CONSTRUCTOR_TSPTWH_HPP

#include "Constructor.hpp"

namespace CVRPTW{
    class Constructor_TSPTWH {
        public:
            virtual bool bestInsertion(routing::models::Solution * solution, routing::DestructionParameters* param);
    };

}


#endif //HYBRID_CONSTRUCTOR_TSPTWH_HPP
