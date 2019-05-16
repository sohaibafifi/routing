//
// Created by ali on 5/5/19.
//

#ifndef HYBRID_WORSTDESTRUCTIONPARAMETER_HPP
#define HYBRID_WORSTDESTRUCTIONPARAMETER_HPP


#include "DestructionParameters.hpp"

namespace routing {
    class WorstDestructionParameters: public routing::DestructionParameters{
        public:
            unsigned long p; //adds randomness for the position of the worst to erase
    };
}


#endif //HYBRID_WORSTDESTRUCTIONPARAMETER_HPP
