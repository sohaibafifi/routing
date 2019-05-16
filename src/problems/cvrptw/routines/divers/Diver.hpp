//
// Created by ali on 5/10/19.
//
#pragma once

#include "../../../../routines/divers/Diver.hpp"

namespace CVRPTW {
    class Diver: public routing::Diver {
    public:
        virtual bool dive(routing::models::Solution* solution, routing::forbiddenPositions* fp ) override;

    };

}
