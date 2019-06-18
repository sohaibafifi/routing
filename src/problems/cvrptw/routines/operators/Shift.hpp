//
// Created by ali on 5/28/19.
//


#pragma once

#include "../../../../routines/neighborhoods/Shift.hpp"

namespace CVRPTW {
    class Shift : public routing::Shift{
        public:
            virtual bool look(routing::models::Solution* solution) override;
            virtual bool doShift(routing::models::Solution* solution, std::pair<int,int> tourPosition) override;
    };
}