//
// Created by ali on 5/3/19.
//

#ifndef HYBRID_SEQUENTIALDESTRUCTIONPARAMETER_HPP
#define HYBRID_SEQUENTIALDESTRUCTIONPARAMETER_HPP

#include "DestructionParameters.hpp"

namespace routing {
    class SequentialDestructionParameters: public routing::DestructionParameters{
    private:
        unsigned long startPosition;
        unsigned long length;
    public:
        unsigned long getStartPosition() const {
            return startPosition;
        }

        void setStartPosition(unsigned long startPosition) {
            SequentialDestructionParameters::startPosition = startPosition;
        }

        unsigned long getLength() const {
            return length;
        }

        void setLength(unsigned long length) {
            SequentialDestructionParameters::length = length;
        }

        SequentialDestructionParameters(unsigned long startPosition, unsigned long length) : startPosition(startPosition),
                                                                                        length(length) {}

    };
}

#endif //HYBRID_SEQUENTIALDESTRUCTIONPARAMETER_HPP
