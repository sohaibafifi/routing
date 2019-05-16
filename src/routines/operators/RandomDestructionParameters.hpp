//
// Created by ali on 5/3/19.
//

#ifndef HYBRID_RANDOMDESTRUCTIONPARAMETER_HPP
#define HYBRID_RANDOMDESTRUCTIONPARAMETER_HPP

#include "DestructionParameters.hpp"
namespace routing {
class RandomDestructionParameters : public routing::DestructionParameters{
        private:
            unsigned long Dmax;

        public:
        RandomDestructionParameters(unsigned long Dmax) : Dmax(Dmax) {}

        unsigned long getDmax() const {
            return Dmax;
        }

        void setDmax(unsigned long Dmax) {
            RandomDestructionParameters::Dmax = Dmax;
        }

    };
}

#endif //HYBRID_RANDOMDESTRUCTIONPARAMETER_HPP
