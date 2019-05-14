//
// Created by ali on 5/10/19.
//
#pragma once

#include "../../../../routines/divers/Diver.hpp"

namespace CVRPTW {
    class Diver: public routing::Diver {
    public:
        virtual bool dive(routing::Problem* problem) override;
        virtual routing::models::Solution* extractPartialSolution(routing::Problem* problem) override;
        virtual routing::forbiddenPositions* extractForbiddenPositions(routing::Problem* problem) override;

    };

}
