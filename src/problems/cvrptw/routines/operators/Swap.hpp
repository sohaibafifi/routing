//
// Created by ali on 6/18/19.
//

#pragma once

#include "../../../../routines/neighborhoods/Swap.hpp"

namespace CVRPTW {
    class Swap : public routing::Swap{
    public:
        virtual bool look(routing::models::Solution* solution) override;
        virtual bool doSwap(routing::models::Solution* solution, std::pair<int,int> tourPosition) override;
    };
}